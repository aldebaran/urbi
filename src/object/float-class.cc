/**
 ** \file object/float-class.cc
 ** \brief Creation of the URBI object float.
 */

#include <cmath>

#include <boost/format.hpp>

#include <sdk/config.h>
#ifndef HAVE_ROUND
#  include <libport/ufloat.h>
#endif

#include <object/atom.hh>
#include <object/float-class.hh>
#include <object/object.hh>
#include <object/string-class.hh>
#include <object/urbi-exception.hh>

namespace object
{
  rObject float_class;

  Float::Float()
    : value_(0)
  {
    proto_add(float_class);
  }

  Float::Float(value_type value)
    : value_(value)
  {
    proto_add(float_class);
  }

  Float::Float(rFloat model)
    : value_(model->value_get())
  {
    proto_add(float_class);
  }

  Float::value_type Float::value_get() const
  {
    return value_;
  }

  Float::value_type& Float::value_get()
  {
    return value_;
  }

  int Float::to_int(const std::string& func) const
  {
    try
    {
      return libport::ufloat_to_int (value_);
    }
    catch (libport::bad_numeric_cast& ue)
    {
      throw BadInteger (value_, func);
      return 0;  // Keep the compiler happy
    }
  }

  rFloat Float::inf()
  {
    return new Float(std::numeric_limits<ufloat>::infinity());
  }

  rFloat Float::nan()
  {
    return new Float(std::numeric_limits<ufloat>::quiet_NaN());
  }

  rString Float::as_string(rObject from)
  {
    if (from.get() == float_class.get())
      return new String(SYMBOL(LT_Float_GT));
    {
      static boost::format f("%g");
      type_check<Float>(from, SYMBOL(asString));
      rFloat fl = from->as<Float>();
      return new String(libport::Symbol(str(f % float(fl->value_get()))));

    }
  }

  rObject Float::operator <(rFloat rhs)
  {
    return value_get() < rhs->value_get() ? true_class : false_class;
  }

#define BOUNCE_OP(Op, Check)                                            \
  rFloat Float::operator Op(rFloat rhs)                                 \
  {                                                                     \
    WHEN(Check, if (!rhs->value_get())                                  \
                  throw PrimitiveError("Operator " #Op,                 \
                                       "division by 0"));               \
    return new Float(value_get() Op rhs->value_get());                  \
  }

  BOUNCE_OP(+, false);
  BOUNCE_OP(*, false);
  BOUNCE_OP(/, true);

#undef BOUNCE_OP

  rFloat Float::operator %(rFloat rhs)
  {
    if (!rhs->value_get())
      throw PrimitiveError("Operator %", "modulo by 0");
    return new Float(fmod(value_get(), rhs->value_get()));
  }

  rFloat Float::pow(rFloat rhs)
  {
    return new Float(powf(value_get(), rhs->value_get()));
  }


#define BOUNCE_INT_OP(Op)                               \
  rFloat Float::operator Op(rFloat rhs)                 \
  {                                                     \
    return new Float(to_int(#Op) Op rhs->to_int(#Op));  \
  }

  BOUNCE_INT_OP(<<);
  BOUNCE_INT_OP(>>);
  BOUNCE_INT_OP(^);

#undef BOUNCE_OP

#define CHECK_POSITIVE(F)                                       \
  if (value_ < 0)                                               \
    throw PrimitiveError(#F, "argument has to be positive");

#define CHECK_TRIGO_RANGE(F)                                    \
  if (value_ < -1 || value_ > 1)                                \
    throw PrimitiveError(#F, "invalid range");

#define BOUNCE_FUN(F, Pos, Range)                       \
  rFloat Float::F()                                     \
  {                                                     \
    WHEN(Pos, CHECK_POSITIVE(F));                       \
    WHEN(Range, CHECK_TRIGO_RANGE(F));                  \
    return new Float(::F(value_get()));                 \
  }

  BOUNCE_FUN(abs, false, false);
  BOUNCE_FUN(acos, false, true);
  BOUNCE_FUN(asin, false, true);
  BOUNCE_FUN(atan, false, false);
  BOUNCE_FUN(cos, false, false);
  BOUNCE_FUN(exp, false, false);
  BOUNCE_FUN(log, true, false);
  BOUNCE_FUN(round, false, false);
  BOUNCE_FUN(sin, false, false);
  BOUNCE_FUN(sqrt, true, false);
  BOUNCE_FUN(tan, false, false);
  BOUNCE_FUN(trunc, false, false);

#undef CHECK_POSITIVE
#undef CHECK_TRIGO_RANGE
#undef BOUNCE_FUN

  rFloat
  Float::random ()
  {
    value_type res = 0.f;
    const long long range = libport::to_long_long (value_get());
    if (range)
      res = rand () % range;
    return new Float(res);
  }

  rFloat Float::minus(objects_type args)
  {
    CHECK_ARG_COUNT_RANGE(0, 1);
    if (args.empty())
      return new Float(-value_get());
    else
    {
      type_check<Float>(args[0], SYMBOL(MINUS));
      return new Float(value_get() - args[0]->as<Float>()->value_get());
    }
  }

  rFloat Float::set(rFloat rhs)
  {
    value_get() = rhs->value_get();
    return this;
  }

  void Float::initialize(CxxObject::Binder<Float>& bind)
  {
    bind(SYMBOL(asString), &Float::as_string);
    bind(SYMBOL(abs), &Float::abs);
    bind(SYMBOL(acos), &Float::acos);
    bind(SYMBOL(asin), &Float::asin);
    bind(SYMBOL(atan), &Float::atan);
    bind(SYMBOL(cos), &Float::cos);
    bind(SYMBOL(exp), &Float::exp);
    bind(SYMBOL(inf), &Float::inf);
    bind(SYMBOL(log), &Float::log);
    bind(SYMBOL(MINUS), &Float::minus);
    bind(SYMBOL(nan), &Float::nan);
    bind(SYMBOL(LT), static_cast<rObject (Float::*)(rFloat)>(&Float::operator<));
    bind(SYMBOL(PLUS), &Float::operator+);
    bind(SYMBOL(STAR), &Float::operator*);
    bind(SYMBOL(SLASH), &Float::operator/);
    bind(SYMBOL(PERCENT), &Float::operator%);
    bind(SYMBOL(LT_LT), &Float::operator<<);
    bind(SYMBOL(GT_GT), &Float::operator>>);
    bind(SYMBOL(CARET), &Float::operator^);
    bind(SYMBOL(STAR_STAR), &Float::pow);
    bind(SYMBOL(random), &Float::random);
    bind(SYMBOL(round), &Float::round);
    bind(SYMBOL(set), &Float::set);
    bind(SYMBOL(sin), &Float::sin);
    bind(SYMBOL(sqrt), &Float::sqrt);
    bind(SYMBOL(tan), &Float::tan);
    bind(SYMBOL(trunc), &Float::trunc);
  }

  bool Float::float_added = CxxObject::add<Float>("Float", float_class);
  const std::string Float::type_name = "Float";
  std::string Float::type_name_get() const
  {
    return type_name;
  }

}; // namespace object

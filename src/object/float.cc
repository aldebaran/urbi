/**
 ** \file object/float-class.cc
 ** \brief Creation of the URBI object float.
 */

#include <boost/format.hpp>

#include <libport/cmath>
#include <libport/compiler.hh>

#include <kernel/config.h>
#ifndef HAVE_ROUND
# include <libport/ufloat.h>
#endif
#include <libport/ufloat.hh>

#include <object/float.hh>
#include <object/list.hh>
#include <object/object.hh>
#include <object/string.hh>
#include <object/symbols.hh>

#include <runner/raise.hh>

namespace object
{
  Float::Float(value_type value)
    : value_(value)
  {
    proto_add(proto ? proto : Object::proto);
  }

  Float::Float(const rFloat& model)
    : value_(model->value_get())
  {
    proto_add(model);
  }

  const Float::value_type&
  Float::value_get() const
  {
    return value_;
  }

  Float::value_type&
  Float::value_get()
  {
    return value_;
  }

#define CONVERSION(Name, Type)				\
  Type							\
  Float::to_##Name(const std::string fmt) const         \
  {							\
    try							\
    {							\
      return libport::numeric_cast<Type>(value_);       \
    }							\
    catch (libport::bad_numeric_cast&)                  \
    {							\
      runner::raise_bad_integer_error(value_, fmt);	\
    }							\
  }							\

  CONVERSION(int, int);
  CONVERSION(unsigned_int, unsigned int);
  CONVERSION(unsigned_type, Float::unsigned_type);
  CONVERSION(long_long, long long);

#undef CONVERSION

  Float::value_type
  Float::inf()
  {
    return std::numeric_limits<libport::ufloat>::infinity();
  }

  Float::value_type
  Float::nan()
  {
    return std::numeric_limits<libport::ufloat>::quiet_NaN();
  }

  std::string
  Float::as_string(int base)
  {
    if (base != 10 && base != 16)
      runner::raise_primitive_error("valid base is 10 or 16");

    // Do not rely on boost::format to print inf and nan since
    // behavior differs under Win32.
    if (std::isinf(value_))
      return value_ > 0 ? SYMBOL(inf) : SYMBOL(MINUS_inf);
    if (std::isnan(value_))
      return SYMBOL(nan);
    // Print integers with a precision ("%.20") in the format string
    // to avoid the scientific notation with loss of precision.
    if (::round(value_) == value_)
    {
      // This format string is enough for a 64 bits integer.
      static boost::format dec("%.20g");
      static boost::format hex("%.20x");
      // Use an integer, otherwise boost::format ignores the
      // hexadecimal flag. Protect the conversion as it may just not fit
      // in a long long. In this case, we will resort to the default
      // display.
      try
      {
	return str((base == 10 ? dec : hex)
                   % libport::numeric_cast<long long>(value_));
      }
      catch (const libport::bad_numeric_cast&)
      {
	// Do nothing.
      }
    }

    if (base == 16)
      runner::raise_bad_integer_error(value_);
    static boost::format f("%g");
    return str(f % value_);
  }

  Float::value_type
  Float::atan2(value_type y)
  {
    return ::atan2(value_, y);
  }

#define BOUNCE_OP(Op, ResType, Check)					\
  ResType								\
  Float::operator Op(value_type rhs)					\
  {                                                                     \
    static libport::Symbol op(#Op);                                     \
    WHEN(Check,                                                         \
         if (!rhs)							\
	   runner::raise_primitive_error("division by 0"));		\
    return value_get() Op rhs;						\
  }

  BOUNCE_OP(*, Float::value_type, false)
  BOUNCE_OP(/, Float::value_type, true)
  BOUNCE_OP(<, bool, false)

#undef BOUNCE_OP

  Float::value_type
  Float::operator %(value_type rhs)
  {
    if (!rhs)
      runner::raise_primitive_error("modulo by 0");
    return fmod(value_get(), rhs);
  }

  Float::value_type
  Float::pow(value_type rhs)
  {
    return powf(value_get(), rhs);
  }


  /*------------------.
  | Unary operators.  |
  `------------------*/
#define BOUNCE_UNSIGNED_OP(Op)                  \
  Float::unsigned_type                          \
  Float::operator Op()				\
  {						\
    return Op to_unsigned_type();               \
  }

  BOUNCE_UNSIGNED_OP(~)
#undef BOUNCE_UNSIGNED_OP


  /*-------------------.
  | Binary operators.  |
  `-------------------*/
#define BOUNCE_UNSIGNED_OP(Op)			\
  Float::unsigned_type                          \
  Float::operator Op(unsigned_type rhs)         \
  {						\
    return to_unsigned_type() Op rhs;           \
  }

  BOUNCE_UNSIGNED_OP(^)
  BOUNCE_UNSIGNED_OP(|)
  BOUNCE_UNSIGNED_OP(&)
  BOUNCE_UNSIGNED_OP(<<)
  BOUNCE_UNSIGNED_OP(>>)
#undef BOUNCE_UNSIGNED_OP

#define CHECK_POSITIVE(F)                                       \
  if (value_ < 0)                                               \
    runner::raise_primitive_error("argument has to be positive");

#define CHECK_TRIGO_RANGE(F)                                    \
  if (value_ < -1 || value_ > 1)                                \
    runner::raise_primitive_error("invalid range");

#define BOUNCE(F, Pos, Range)                           \
  Float::value_type					\
  Float::F()                                            \
  {                                                     \
    WHEN(Pos, CHECK_POSITIVE(SYMBOL(F)));               \
    WHEN(Range, CHECK_TRIGO_RANGE(SYMBOL(F)));          \
    return ::F(value_get());				\
  }

  BOUNCE(acos,  false, true);
  BOUNCE(asin,  false, true);
  BOUNCE(atan,  false, false);
  BOUNCE(cos,   false, false);
  BOUNCE(exp,   false, false);
  BOUNCE(fabs,  false, false);
  BOUNCE(log,   true,  false);
  BOUNCE(round, false, false);
  BOUNCE(sin,   false, false);
  BOUNCE(sqrt,  true,  false);
  BOUNCE(tan,   false, false);
  BOUNCE(trunc, false, false);

#undef CHECK_POSITIVE
#undef CHECK_TRIGO_RANGE
#undef BOUNCE

  int
  Float::random()
  {
    static const std::string fmt = "expected positive integer, got %s";
    unsigned int upper_bound = to_unsigned_int(fmt);
    if (!upper_bound)
      runner::raise_bad_integer_error(value_get(), fmt);
    return rand() % upper_bound;
  }

  rFloat
  Float::plus(const objects_type& args)
  {
    check_arg_count(args.size(), 0, 1);
    if (args.empty())
      return this;
    else
    {
      type_check<Float>(args[0], 1);
      return new Float(value_get() + args[0]->as<Float>()->value_get());
    }
  }

  rFloat
  Float::minus(const objects_type& args)
  {
    check_arg_count(args.size(), 0, 1);
    if (args.empty())
      return new Float(-value_get());
    else
    {
      type_check<Float>(args[0], 1);
      return new Float(value_get() - args[0]->as<Float>()->value_get());
    }
  }

  rList
  Float::seq()
  {
    unsigned int n = to_unsigned_int();
    List::value_type res;
    for (unsigned int i = 0; i < n; i++)
      res.push_back(new Float(i));
    return new List(res);
  }

  /*--------.
  | Details |
  `--------*/

  std::ostream& Float::special_slots_dump(std::ostream& o) const
  {
    o << "value = " << value_ << libport::iendl;
    return o;
  }


  /*---------------.
  | Binding system |
  `---------------*/

  OVERLOAD_DEFAULT(as_string_bouncer, 0, &Float::as_string, 10);

  URBI_CXX_OBJECT_REGISTER(Float);

  void
  Float::initialize(CxxObject::Binder<Float>& bind)
  {
    bind(SYMBOL(CARET), &Float::operator^);
    bind(SYMBOL(GT_GT), &Float::operator>>);
    bind(SYMBOL(LT), static_cast<bool (Float::*)(value_type)>(&Float::operator<));
    bind(SYMBOL(LT_LT), &Float::operator<<);
    bind(SYMBOL(MINUS), &Float::minus);
    bind(SYMBOL(PERCENT), &Float::operator%);
    bind(SYMBOL(PLUS), &Float::plus);
    bind(SYMBOL(SLASH), &Float::operator/);
    bind(SYMBOL(STAR), &Float::operator*);
    bind(SYMBOL(STAR_STAR), &Float::pow);
    bind(SYMBOL(abs), &Float::fabs);
    bind(SYMBOL(acos), &Float::acos);
    bind(SYMBOL(asString), &as_string_bouncer);
    bind(SYMBOL(asin), &Float::asin);
    bind(SYMBOL(atan), &Float::atan);
    bind(SYMBOL(atan2), &Float::atan2);
    bind(SYMBOL(bitand), &Float::operator&);
    bind(SYMBOL(bitor), &Float::operator|);
    bind(SYMBOL(compl), &Float::operator~);
    bind(SYMBOL(cos), &Float::cos);
    bind(SYMBOL(exp), &Float::exp);
    bind(SYMBOL(inf), &Float::inf);
    bind(SYMBOL(log), &Float::log);
    bind(SYMBOL(nan), &Float::nan);
    bind(SYMBOL(random), &Float::random);
    bind(SYMBOL(round), &Float::round);
    bind(SYMBOL(seq), &Float::seq);
    bind(SYMBOL(sin), &Float::sin);
    bind(SYMBOL(sqrt), &Float::sqrt);
    bind(SYMBOL(tan), &Float::tan);
    bind(SYMBOL(trunc), &Float::trunc);
  }

  rObject
  Float::proto_make()
  {
    return new Float(0);
  }

}; // namespace object

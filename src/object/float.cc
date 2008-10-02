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
#include <object/urbi-exception.hh>

namespace object
{
  Float::Float()
    : value_(0)
  {
    proto_add(proto);
  }

  Float::Float(value_type value)
    : value_(value)
  {
    proto_add(proto);
  }

  Float::Float(const rFloat& model)
    : value_(model->value_get())
  {
    proto_add(model);
    proto_remove(proto);
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

  int
  Float::to_int(const libport::Symbol func) const
  {
    try
    {
      return libport::ufloat_to_int(value_);
    }
    catch (libport::bad_numeric_cast& ue)
    {
      throw BadInteger(value_, func);
      return 0;  // Keep the compiler happy
    }
  }

  unsigned int
  Float::to_unsigned_int(const libport::Symbol func,
			 const std::string fmt) const
  {
    try
    {
      int res = libport::ufloat_to_int(value_);
      if (res < 0)
	throw libport::bad_numeric_cast();
      return res;
    }
    catch (libport::bad_numeric_cast& ue)
    {
      throw BadInteger(value_, func, fmt);
    }
  }

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
  Float::as_string(const rObject& from)
  {
    if (from == proto)
      return SYMBOL(LT_Float_GT);

    type_check(from, proto, 1);
    Float::value_type fl = from->as<Float>()->value_get();
    // Do not rely on boost::format to print inf and nan since
    // behavior differs under Win32.
    if (std::isinf(fl))
      return fl > 0 ? SYMBOL(inf) : SYMBOL(MINUS_inf);
    if (std::isnan(fl))
      return SYMBOL(nan);
    // Print integers with a different format string to avoid
    // the scientific notation with loss of precision.
    if (::round(fl) == fl)
    {
      // This format string is enough for a 64 bits integer.
      static boost::format f("%.20g");
      return str(f % fl);
    }
    else
    {
      static boost::format f("%g");
      return str(f % fl);
    }
  }

#define BOUNCE_OP(Op, ResType, Check)					\
  ResType								\
  Float::operator Op(value_type rhs)					\
  {                                                                     \
    static libport::Symbol op(#Op);                                     \
    WHEN(Check,                                                         \
         if (!rhs)							\
           throw PrimitiveError(op, "division by 0"));                  \
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
      throw PrimitiveError(SYMBOL(PERCENT), "modulo by 0");
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
#define BOUNCE_INT_OP(Op)			\
  int						\
  Float::operator Op()				\
  {						\
    static libport::Symbol op(#Op);		\
    return Op to_int(op);			\
  }
BOUNCE_INT_OP(~)
#undef BOUNCE_INT_OP


  /*-------------------.
  | Binary operators.  |
  `-------------------*/
#define BOUNCE_INT_OP(Op)			\
  int						\
  Float::operator Op(int rhs)			\
  {						\
    static libport::Symbol op(#Op);		\
    return to_int(op) Op rhs;			\
}

  BOUNCE_INT_OP(^)
  BOUNCE_INT_OP(|)
  BOUNCE_INT_OP(&)
#undef BOUNCE_INT_OP

#define BOUNCE_UNSIGNED_INT_OP(Op)		\
  int						\
  Float::operator Op(unsigned int rhs)		\
  {						\
    static libport::Symbol op(#Op);		\
    return to_int(op) Op rhs;			\
}

  BOUNCE_UNSIGNED_INT_OP(<<)
  BOUNCE_UNSIGNED_INT_OP(>>)
#undef BOUNCE_UNSIGNED_INT_OP

#define CHECK_POSITIVE(F)                                       \
  if (value_ < 0)                                               \
    throw PrimitiveError(F, "argument has to be positive");

#define CHECK_TRIGO_RANGE(F)                                    \
  if (value_ < -1 || value_ > 1)                                \
    throw PrimitiveError(F, "invalid range");

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
    static const std::string fmt = "expected positive integer, got %1%";
    unsigned int upper_bound = to_unsigned_int(SYMBOL(random), fmt);
    if (!upper_bound)
      throw BadInteger(value_get(), SYMBOL(random), fmt);
    return rand() % upper_bound;
  }

  rFloat
  Float::plus(objects_type& args)
  {
    check_arg_count(args.size(), 0, 1);
    if (args.empty())
      return this;
    else
    {
      type_check(args[0], proto, 1);
      return new Float(value_get() + args[0]->as<Float>()->value_get());
    }
  }

  rFloat
  Float::minus(objects_type& args)
  {
    check_arg_count(args.size(), 0, 1);
    if (args.empty())
      return new Float(-value_get());
    else
    {
      type_check(args[0], proto, 1);
      return new Float(value_get() - args[0]->as<Float>()->value_get());
    }
  }

  rList
  Float::seq()
  {
    unsigned int n = to_unsigned_int(SYMBOL(seq));
    List::value_type res;
    for (unsigned int i = 0; i < n; i++)
      res.push_back(new Float(i));
    return new List(res);
  }

  rFloat
  Float::set(value_type rhs)
  {
    value_ = rhs;
    return this;
  }

  /*--------.
  | Details |
  `--------*/

  std::ostream& Float::special_slots_dump(std::ostream& o,
                                          runner::Runner&) const
  {
    o << "value = " << value_ << libport::iendl;
    return o;
  }


  /*---------------.
  | Binding system |
  `---------------*/

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
    bind(SYMBOL(asString), &Float::as_string);
    bind(SYMBOL(asin), &Float::asin);
    bind(SYMBOL(atan), &Float::atan);
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
    bind(SYMBOL(set), &Float::set);
    bind(SYMBOL(sin), &Float::sin);
    bind(SYMBOL(sqrt), &Float::sqrt);
    bind(SYMBOL(tan), &Float::tan);
    bind(SYMBOL(trunc), &Float::trunc);
  }

  bool Float::float_added = CxxObject::add<Float>(Float::proto);
  const std::string Float::type_name = "Float";
  rObject Float::proto;
  std::string
  Float::type_name_get() const
  {
    return type_name;
  }

}; // namespace object

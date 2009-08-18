/**
 ** \file object/float-class.cc
 ** \brief Creation of the URBI object float.
 */

#include <algorithm>

#include <libport/cmath>
#include <libport/compiler.hh>
#include <libport/format.hh>

#include <kernel/config.h>
#ifndef HAVE_ROUND
# include <libport/ufloat.h>
#endif
#include <libport/ufloat.hh>

#include <object/float.hh>
#include <object/format-info.hh>
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


  bool
  Float::operator==(const rObject& rhs) const
  {
    return (rhs->is_a<Float>()
            && *this == rhs->as<Float>()->value_get());
  }

  bool
  Float::operator==(const value_type& rhs) const
  {
    return value_get() == rhs;
  }

  bool
  Float::operator<=(const value_type& rhs) const
  {
    return value_get() <= rhs;
  }


  /*--------------.
  | Conversions.  |
  `--------------*/

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

  bool
  Float::as_bool() const
  {
    return value_;
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
  Float::format(rFormatInfo finfo) const
  {
    std::string pattern(finfo->pattern_get());

    replace(pattern.begin(), pattern.end(), 's', 'g');
    replace(pattern.begin(), pattern.end(), 'd', 'g');
    replace(pattern.begin(), pattern.end(), 'D', 'G');
    try
    {
      return libport::format(pattern,
                             libport::numeric_cast<long long>(value_));
    }
    catch (const libport::bad_numeric_cast&)
    {
      // Do nothing.
    }

    if (finfo->spec_get() == "x" || finfo->spec_get() == "o")
      runner::raise_bad_integer_error(value_);
    return libport::format(pattern, value_);
  }

  std::string
  Float::as_string() const
  {
    try
    {
      return libport::format("%1%",
                             libport::numeric_cast<long long>(value_));
    }
    catch (const libport::bad_numeric_cast&)
    {
      return libport::format("%1%", value_);
    }
  }

  Float::value_type
  Float::atan2(value_type y) const
  {
    return ::atan2(value_, y);
  }

#define BOUNCE_OP(Op, ResType, Check)					\
  ResType								\
  Float::operator Op(value_type rhs) const				\
  {                                                                     \
    static libport::Symbol op(#Op);                                     \
    WHEN(Check,                                                         \
         if (!rhs)							\
	   RAISE("division by 0"));                                     \
    return value_get() Op rhs;						\
  }

  BOUNCE_OP(*, Float::value_type, false)
  BOUNCE_OP(/, Float::value_type, true)

#undef BOUNCE_OP

  Float::value_type
  Float::operator %(value_type rhs) const
  {
    if (!rhs)
      RAISE("modulo by 0");
    return fmod(value_get(), rhs);
  }

  Float::value_type
  Float::pow(value_type rhs) const
  {
    return powf(value_get(), rhs);
  }


  /*------------------.
  | Unary operators.  |
  `------------------*/
#define BOUNCE_UNSIGNED_OP(Op)                  \
  Float::unsigned_type                          \
  Float::operator Op() const                    \
  {						\
    return Op to_unsigned_type();               \
  }

  BOUNCE_UNSIGNED_OP(~)
#undef BOUNCE_UNSIGNED_OP


  /*-------------------.
  | Binary operators.  |
  `-------------------*/
#define BOUNCE_UNSIGNED_OP(Op)                  \
  Float::unsigned_type                          \
  Float::operator Op(unsigned_type rhs) const   \
  {						\
    return to_unsigned_type() Op rhs;           \
  }

  BOUNCE_UNSIGNED_OP(^)
  BOUNCE_UNSIGNED_OP(|)
  BOUNCE_UNSIGNED_OP(&)
  BOUNCE_UNSIGNED_OP(<<)
  BOUNCE_UNSIGNED_OP(>>)
#undef BOUNCE_UNSIGNED_OP

#define CHECK_POSITIVE(F)                       \
  do {                                          \
    if (value_ < 0)                             \
      RAISE("argument has to be positive");     \
  } while (0)

#define CHECK_TRIGO_RANGE(F)                    \
  do {                                          \
    if (value_ < -1 || 1 < value_)              \
      RAISE("invalid range");                   \
  } while (0)

#define BOUNCE(F, Pos, Range)                           \
  Float::value_type					\
  Float::F() const                                      \
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
  Float::random() const
  {
    static const std::string fmt = "expected positive integer, got %s";
    unsigned int upper_bound = to_unsigned_int(fmt);
    if (!upper_bound)
      runner::raise_bad_integer_error(value_get(), fmt);
    return rand() % upper_bound;
  }


#define DEFINE(Urbi, Op)                                        \
  Float::value_type                                             \
  Float::Urbi(const objects_type& args) const                   \
  {                                                             \
    check_arg_count(args.size(), 0, 1);                         \
    if (args.empty())                                           \
      return Op value_get();                                    \
    else                                                        \
    {                                                           \
      type_check<Float>(args[0], 1);                            \
      return value_get() Op args[0]->as<Float>()->value_get();  \
    }                                                           \
  }

  DEFINE(plus, +);
  DEFINE(minus, -);
#undef DEFINE


  rList
  Float::seq() const
  {
    unsigned int n = to_unsigned_int();
    List::value_type res;
    for (unsigned int i = 0; i < n; i++)
      res.push_back(new Float(i));
    return new List(res);
  }

  /*----------.
  | Details.  |
  `----------*/

  std::ostream& Float::special_slots_dump(std::ostream& o) const
  {
    o << "value = " << value_ << libport::iendl;
    return o;
  }


  /*-----------------.
  | Binding system.  |
  `-----------------*/

  URBI_CXX_OBJECT_REGISTER(Float);

  void
  Float::initialize(CxxObject::Binder<Float>& bind)
  {
    bind(SYMBOL(EQ_EQ),
         static_cast<bool (Float::*)(const rObject&) const>
                    (&Float::operator==));
#define DECLARE(Urbi, Cxx)                      \
    bind(SYMBOL(Urbi), &Float::Cxx)

    DECLARE(CARET, operator^);
    DECLARE(GT_GT, operator>>);
    DECLARE(LT_EQ, operator<=);
    DECLARE(LT_LT, operator<<);
    DECLARE(MINUS, minus);
    DECLARE(PERCENT, operator%);
    DECLARE(PLUS, plus);
    DECLARE(SLASH, operator/);
    DECLARE(STAR, operator*);
    DECLARE(STAR_STAR, pow);
    DECLARE(abs, fabs);
    DECLARE(acos, acos);
    DECLARE(asString, as_string);
    DECLARE(asin, asin);
    DECLARE(atan, atan);
    DECLARE(atan2, atan2);
    DECLARE(bitand, operator&);
    DECLARE(bitor, operator|);
    DECLARE(compl, operator~);
    DECLARE(cos, cos);
    DECLARE(exp, exp);
    DECLARE(format, format);
    DECLARE(inf, inf);
    DECLARE(log, log);
    DECLARE(nan, nan);
    DECLARE(random, random);
    DECLARE(round, round);
    DECLARE(seq, seq);
    DECLARE(sin, sin);
    DECLARE(sqrt, sqrt);
    DECLARE(tan, tan);
    DECLARE(trunc, trunc);

#undef DECLARE
  }

  rObject
  Float::proto_make()
  {
    return new Float(0);
  }

}; // namespace object

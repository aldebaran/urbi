/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/object/vector.hh>
#include <urbi/object/matrix.hh>
#include <kernel/uvalue-cast.hh>
#include <urbi/uvalue.hh>
#include <boost/numeric/ublas/lu.hpp> // boost::numeric::ublas::row

namespace boost
{
  namespace numeric
  {
    namespace ublas
    {

      /*--------------.
      | vector_type.  |
      `--------------*/

      bool
      operator==(const ::urbi::object::vector_type& e1,
                 const ::urbi::object::vector_type& e2)
      {
        if (e1.size() != e2.size())
          return false;
        for (unsigned i = 0; i < e1.size(); ++i)
          if (e1(i) != e2(i))
            return false;
        return true;
      }

      std::ostream&
      operator<<(std::ostream& o, const ::urbi::object::vector_type& v)
      {
        o << '<';
        for (unsigned i = 0; i < v.size(); ++i)
        {
          if (i)
            o << ", ";
          o << v(i);
        }
        return o << '>';
      }



      /*--------------.
      | matrix_type.  |
      `--------------*/

      bool
      operator==(const ::urbi::object::matrix_type& e1,
                 const ::urbi::object::matrix_type& e2)
      {
        return
          (e1.size1() == e2.size1()
           && e1.size2() == e2.size2()
           && norm_inf(e1 - e2) == 0);
      }

      std::ostream&
      operator<<(std::ostream& o, const ::urbi::object::matrix_type& v)
      {
        const size_t height = v.size1();
        o << '<';
        for (unsigned i = 0; i < height; ++i)
        {
          if (i)
            o << ", ";
          o << boost::numeric::ublas::row(v, i);
        }
        return o << '>';
      }

    }
  }
}

namespace urbi
{
  namespace object
  {

    /*---------.
    | Vector.  |
    `---------*/

    Vector::Vector()
    {
      proto_add(proto);
    }

    Vector::Vector(const rVector& model)
      : value_(model->value_)
    {
      proto_add(proto);
    }

    Vector::Vector(const value_type& model)
      : value_(model)
    {
      proto_add(proto);
    }

    Vector::Vector(const objects_type& model)
      : value_(model.size())
    {
      proto_add(proto);
      fromList(model);
    }

    Vector::Vector(size_t s)
      : value_(s)
    {
      proto_add(proto);
    }

    rVector
    Vector::init(const objects_type& args)
    {
      rVector self = args[0]->as<Vector>();
      if (!self)
        runner::raise_type_error(args[1], Vector::proto);
      if (args.size() == 2)
      {
        if (rList l = args[1]->as<List>())
          return self->fromList(l->value_get());
        else if (rVector v = args[1]->as<Vector>())
        {
          self->value_ = v->value_;
          return self;
        }
      }
      self->value_.resize(args.size() - 1);
      for (unsigned i = 1; i < args.size(); ++i)
        self->value_(i-1) = from_urbi<ufloat>(args[i]);
      return self;
    }

    Vector*
    Vector::fromList(const objects_type& model)
    {
      size_t size = model.size();
      value_.resize(size);
      for (unsigned i = 0; i < size; ++i)
        value_(i) = from_urbi<ufloat>(model[i]);
      return this;
    }

    URBI_CXX_OBJECT_INIT(Vector)
    {
      BIND(PLUS, operator+, Vector*, ());
      BIND(PLUS, operator+, value_type, (const rObject&) const);
      /* Something is wrong with the handling of these two overloads.
       * so use a disambiguator.
      BIND(PLUS, operator+, value_type, (const value_type&));
      BIND(PLUS, operator+, value_type, (ufloat));
           */
      BIND(MINUS, operator-, value_type, ());
      BIND(MINUS, operator-, value_type, (const rObject&) const);
      /*
      BIND(MINUS, operator-, value_type, (const value_type&));
      BIND(MINUS, operator-, value_type, (ufloat));
      */
      BIND(STAR, operator*, value_type, (const rObject&) const);
      BIND(SLASH, operator/, value_type, (const rObject&) const);

      BIND(EQ_EQ, operator==, bool, (const rObject&) const);
      BIND(LT, operator<, bool, (const Object&) const);

      BIND(SBL_SBR, operator[]);
      BIND(SBL_SBR_EQ, set);
      BIND(asString, as_string);
      BIND(combAdd);
      BIND(combDiv);
      BIND(combMul);
      BIND(combSub);
      BIND(distance);
      BIND(norm);
      BIND(resize);
      BIND(scalarGE);
      BIND(scalarLE);
      BIND(set, fromList);
      BIND(size);
      BIND(sum);
      BIND(trueIndexes);
      BIND(uvalueSerialize);
      bind(libport::Symbol("scalar" "EQ"), &Vector::scalarEQ);
      bind(libport::Symbol("scalar" "GT"), &Vector::scalarGT);
      bind(libport::Symbol("scalar" "LT"), &Vector::scalarLT);
      slot_set(SYMBOL(init), new Primitive(&init));
    }

    std::string
    Vector::as_string() const
    {
      std::ostringstream s;
      s << '<';
      for (unsigned i = 0; i < size(); ++i)
      {
        if (i)
          s << ", ";
        s << value_(i);
      }
      s << '>';
      return s.str();
    }

    ufloat
    Vector::operator[](int i) const
    {
      return value_(index(i));
    }

    ufloat
    Vector::set(int i, ufloat v)
    {
      return value_(index(i)) = v;
    }

    size_t
    Vector::size() const
    {
      return value_.size();
    }

    ufloat
    Vector::norm() const
    {
      return norm_2(value_);
    }

    ufloat
    Vector::distance(const value_type& that) const
    {
      return norm_2(*this - that);
    }

    size_t
    Vector::index(int i) const
    {
      int res = i;
      if (res < 0)
        res += size();
      if (res < 0 || size() <= size_t(res))
        FRAISE("invalid index: %s", i);
      return res;
    }

#define OP(Name, Op)                                            \
    Vector::value_type                                          \
    Vector::Name(const value_type &b) const                     \
    {                                                           \
      size_t s1 = size();                                       \
      size_t s2 = b.size();                                     \
      if (s1 != s2)                                             \
        FRAISE("incompatible vector sizes: %s, %s", s1, s2);    \
      value_type res(s1);                                       \
      for (unsigned i = 0; i < s1; ++i)                         \
        res(i) = value_(i) Op b(i);                             \
      return res;                                               \
    }

    OP(scalarGT, >)
    OP(scalarLT, <)
    OP(scalarLE, <=)
    OP(scalarGE, >=)
    OP(scalarEQ, ==)
    OP(operator *, *)
    OP(operator /, /)
#undef OP

    Vector::value_type
    Vector::trueIndexes() const
    {
      unsigned count = 0;
      for (unsigned i = 0; i < size(); ++i)
        if (value_(i))
          ++count;
      value_type res(count);
      unsigned pos = 0;
      for (unsigned i = 0; i < size(); ++i)
        if (value_(i))
          res(pos++) = i;
      return res;
    }

#define OP(Name, Op)                                    \
    Vector::matrix_type                                 \
    Vector::comb ## Name(const value_type& b) const     \
    {                                                   \
      size_t s1 = size();                               \
      size_t s2 = b.size();                             \
      matrix_type res(s1, s2);                          \
      for (unsigned i = 0; i < s1; ++i)                 \
        for (unsigned j = 0; j < s2; ++j)               \
          res(i, j) = value_(i) Op b(j);                \
      return res;                                       \
    }

    OP(Add, +)
    OP(Div, /)
    OP(Mul, *)
    OP(Sub, -)
#undef OP

    Vector*
    Vector::resize(size_t s)
    {
      value_.resize(s);
      return this;
    }

#define OP(Op)                                                          \
    Vector::value_type                                                  \
    Vector::operator Op (const rObject& b) const                        \
    {                                                                   \
      if (rVector v = b->as<Vector>())                                  \
        return operator Op(v->value_get());                             \
      else                                                              \
        return operator Op(b->as_checked<Float>()->value_get());        \
    }

    OP(+)
    OP(-)
    OP(*)
    OP(/)
#undef OP

    ufloat
    Vector::sum() const
    {
      ufloat res = 0;
      for (unsigned i = 0; i < size(); ++i)
        res += value_(i);
      return res;
    }

    bool
    Vector::operator<(const value_type& b) const
    {
      for (size_t i = 0; i < std::min(size(), b.size()); ++i)
        if (value_(i) != b(i))
          return value_(i) < b(i);
      return size() < b.size();
    }

    rObject
    Vector::uvalueSerialize() const
    {
      CAPTURE_GLOBAL(Binary);
      // This is ugly: we cannot go through object-cast as it would give us
      // back the vector. So make the Binary ourselve.
      rObject o(const_cast<Vector*>(this));
      urbi::UValue v = ::uvalue_cast(o);
      rObject res = new object::Object();
      res->proto_add(Binary);
      res->slot_set(SYMBOL(keywords),
                    new object::String(v.binary->getMessage()));
      res->slot_set(SYMBOL(data),
                    new object::String
                    (std::string(static_cast<char*>(v.binary->common.data),
				 v.binary->common.size)));
      return res;
    }
  }
}

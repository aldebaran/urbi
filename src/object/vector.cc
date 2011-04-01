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

namespace boost
{
  namespace numeric
  {
    namespace ublas
    {
      inline
      bool operator==(const vector< ::libport::ufloat >& v1,
                      const vector< ::libport::ufloat >& v2)
      {
        if (v1.size() != v2.size())
          return false;
	for (unsigned i = 0; i < v1.size(); ++i)
	  if (v1(i) != v2(i))
            return false;
	return true;
      }
    }
  }
}

namespace urbi
{
  namespace object
  {
    Vector::Vector()
    {
      proto_add(proto);
    }

    Vector::Vector(value_type v)
      : value_(v)
    {
      proto_add(proto);
    }

    Vector::Vector(const Vector& model)
      : CxxObject()
      , super_comparable_type()
      , value_(model.value_)
    {
      proto_add(proto);
    }

    Vector::Vector(const rVector& model)
      : value_(model->value_)
    {
      proto_add(proto);
    }

    Vector::Vector(const rList& model)
      : value_(model->size())
    {
      proto_add(proto);
      fromList(model);
    }

    Vector::Vector(unsigned int s)
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
          return self->fromList(l);
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

    rVector
    Vector::fromList(const rList& model)
    {
      value_.resize(model->size());
      for(unsigned i=0; i<model->size(); ++i)
        value_(i) = from_urbi<ufloat>(model->value_get()[i]);
      return this;
    }

    URBI_CXX_OBJECT_INIT(Vector)
    {
      bind(SYMBOL(PLUS),
          static_cast<rVector (Vector::*)()>(&Vector::operator+));
      bind(SYMBOL(PLUS),
           static_cast<rVector (Vector::*)(const rObject&) const>
           (&Vector::operator+));
      /* Something is wrong with the handling of these two overloads.
       * so use a disambiguator.
      bind(SYMBOL(PLUS),
           static_cast<rVector (Vector::*)(const rVector&)>
           (&Vector::operator+));
      bind(SYMBOL(PLUS),
           static_cast<rVector (Vector::*)(ufloat)>
           (&Vector::operator+));
           */
      bind(SYMBOL(MINUS),
           static_cast<rVector (Vector::*)()>(&Vector::operator-));
      bind(SYMBOL(MINUS),
           static_cast<rVector (Vector::*)(const rObject&) const>
           (&Vector::operator-));
      /*
      bind(SYMBOL(MINUS),
           static_cast<rVector (Vector::*)(const rVector&)>
           (&Vector::operator-));
      bind(SYMBOL(MINUS),
           static_cast<rVector (Vector::*)(ufloat)>
           (&Vector::operator-));
      */
      bind(SYMBOL(STAR),
           static_cast<rVector (Vector::*)(const rObject&) const>
           (&Vector::operator*));
      bind(SYMBOL(SLASH),
           static_cast<rVector (Vector::*)(const rObject&) const>
           (&Vector::operator/));

      bind(SYMBOL(EQ_EQ),
           static_cast<bool (self_type::*)(const rObject&) const>
           (&self_type::operator==));

#define DECLARE(Name, Fun)                      \
      bind(SYMBOL_(Name), &self_type::Fun)

      DECLARE(LT, operator<);
      DECLARE(SBL_SBR, operator[]);
      DECLARE(SBL_SBR_EQ, set);
      DECLARE(asString, as_string);
      DECLARE(combAdd, combAdd);
      DECLARE(combDiv, combDiv);
      DECLARE(combMul, combMul);
      DECLARE(norm, norm);
      DECLARE(resize, resize);
      DECLARE(scalarGE, scalarGE);
      DECLARE(scalarLE, scalarLE);
      DECLARE(selfCombAdd, selfCombAdd);
      DECLARE(selfCombDiv, selfCombDiv);
      DECLARE(selfCombIndexes, selfCombIndexes);
      DECLARE(selfCombMul, selfCombMul);
      DECLARE(set, fromList);
      DECLARE(size, size);
      DECLARE(sum, sum);
      DECLARE(trueIndexes, trueIndexes);
#undef DECLARE
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
      for (unsigned i=0; i<value_.size(); ++i)
      {
        s << value_(i);
        if (i != value_.size()-1)
          s << ", ";
      }
      s << '>';
      return s.str();
    }

    ufloat
    Vector::operator[](int i) const
    {
      return value_(index(i));
    }

    rVector
    Vector::set(int i, ufloat v)
    {
      value_(index(i)) = v;
      return this;
    }

    unsigned int
    Vector::size() const
    {
      return value_.size();
    }

    ufloat
    Vector::norm() const
    {
      ufloat res = 0;
      for (unsigned i=0; i<value_.size(); ++i)
        res += value_(i)*value_(i);
      return sqrt(res);
    }

    unsigned int
    Vector::index(int i) const
    {
      if (i < 0)
        i += value_.size();
      if (i < 0 || value_.size() <= (unsigned int)i)
        FRAISE("invalid index: %s", i);
      return i;
    }

#define OP(Name, Op)                            \
    rVector                                     \
    Vector::Name(const rVector &bv) const       \
    {                                           \
      Vector::value_type& b = bv->value_get();  \
      rVector res(new Vector(size()));          \
      Vector::value_type& v = res->value_get(); \
      for (unsigned i=0; i<size(); ++i)         \
        v(i) = value_(i) Op b(i);               \
      return res;                               \
    }

    OP(scalarGT, >)
    OP(scalarLT, <)
    OP(scalarLE, <=)
    OP(scalarGE, >=)
    OP(scalarEQ, ==)
    OP(operator *, *)
    OP(operator /, /)
#undef OP

    rVector
    Vector::trueIndexes() const
    {
      unsigned count = 0;
      for (unsigned i=0; i<size(); ++i)
        if (value_(i)) ++count;
      rVector res(new Vector(count));
      Vector::value_type& v = res->value_get();
      unsigned pos = 0;
      for (unsigned i=0; i<size(); ++i)
        if (value_(i)) v(pos++) = i;
      return res;
    }

#define OP(Name, Op)                                    \
    rVector                                             \
    Vector::comb ## Name(rVector vb) const              \
    {                                                   \
      Vector::value_type& b = vb->value_get();          \
      rVector res(new Vector(size() * b.size()));       \
      Vector::value_type& v = res->value_get();         \
      for (unsigned i = 0; i < size(); ++i)             \
        for (unsigned j = 0; j < b.size(); ++j)         \
          v(i * b.size() + j) = value_(i) Op b(j);      \
      return res;                                       \
    }

    OP(Add, +)
    OP(Mul, *)
    OP(Div, /)
#undef OP

#define OP(Name, Op)                                    \
    rVector                                             \
    Vector::selfComb ## Name() const                    \
    {                                                   \
      rVector res(new Vector(size() * (size()-1) / 2)); \
      Vector::value_type& v = res->value_get();         \
      unsigned p = 0;                                   \
      for (unsigned i = 0; i < size(); ++i)             \
        for (unsigned j = i+1; j < size(); ++j)         \
          v(p++) = value_(i) Op value_(j);              \
      return res;                                       \
    }

    OP(Add, +)
    OP(Div, /)
    OP(Mul, *)
#undef OP

    std::pair<unsigned int, unsigned int>
    Vector::selfCombIndexes(unsigned int v)
    {
      unsigned int i=0;
      unsigned int j=0;
      unsigned int p=0;
      while (p+ size()-i -1 < v)
      {
        p += size() -i - 1;
        ++i;
      }
      j = v-p;
      return std::make_pair(i, j);
    }

    void
    Vector::resize(unsigned int s)
    {
      value_.resize(s);
    }

#define OP(Op)                                                  \
    rVector                                                     \
    Vector::operator Op (const rObject& b) const                \
    {                                                           \
      if (rVector v = b->as<Vector>())                          \
        return (*this) Op v;                                    \
      else                                                      \
        return (*this) Op b->as_checked<Float>()->value_get();  \
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
      for (unsigned i=0; i<size(); ++i)
        res += value_(i);
      return res;
    }
  }
}

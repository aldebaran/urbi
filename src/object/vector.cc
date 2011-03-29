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

    rObject
    Vector::init(const objects_type& args)
    {
      rVector self = args[0]->as<Vector>();
      if (!self)
        runner::raise_type_error(args[1], Vector::proto);
      if (args.size() == 2)
      {
        if (rList l = args[1]->as<List>())
        {
          self->fromList(l);
          return self;
        }
        else if (rVector v = args[1]->as<Vector>())
        {
          self->value_ = v->value_;
          return self;
        }
      }
      self->value_.resize(args.size()-1);
      for (unsigned i = 1; i<args.size(); ++i)
        if (rFloat f = args[i]->as<Float>())
          self->value_(i-1) = f->value_get();
        else
          runner::raise_type_error(args[i], Float::proto);
      return self;
    }

    rObject
    Vector::fromList(const rList& model)
    {
      value_.resize(model->size());
      for(unsigned i=0; i<model->size(); ++i)
        value_(i) = model->value_get()[i]->as<Float>()->value_get();
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
      bind(SYMBOL(SBL_SBR), &Vector::operator[]);
      bind(SYMBOL(SBL_SBR_EQ), &Vector::set);
      bind(SYMBOL(LT), &Vector::operator<);
      bind(SYMBOL(set), &Vector::fromList);
      bind(SYMBOL(size), &Vector::size);
      bind(SYMBOL(resize), &Vector::resize);
      bind(SYMBOL(norm), &Vector::norm);
      bind(libport::Symbol("scalar" "GT"), &Vector::scalarGT);
      bind(libport::Symbol("scalar" "LT"), &Vector::scalarLT);
      bind(SYMBOL(scalarGE), &Vector::scalarGE);
      bind(SYMBOL(scalarLE), &Vector::scalarLE);
      bind(libport::Symbol("scalar" "EQ"), &Vector::scalarEQ);
      bind(SYMBOL(trueIndexes), &Vector::trueIndexes);
      bind(SYMBOL(combAdd), &Vector::combAdd);
      bind(SYMBOL(combMul), &Vector::combMul);
      bind(SYMBOL(combDiv), &Vector::combDiv);
      bind(SYMBOL(selfCombAdd), &Vector::selfCombAdd);
      bind(SYMBOL(selfCombMul), &Vector::selfCombMul);
      bind(SYMBOL(selfCombDiv), &Vector::selfCombDiv);
      bind(SYMBOL(selfCombIndexes), &Vector::selfCombIndexes);
      bind(SYMBOL(sum), &Vector::sum);
      slot_set(SYMBOL(init), new Primitive(&init));
      bind(SYMBOL(asString), &Vector::as_string);
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

    rObject
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

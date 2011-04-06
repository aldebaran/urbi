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
#include <kernel/uvalue-cast.hh>
#include <urbi/uvalue.hh>

namespace urbi
{
  namespace object
  {
    bool
    operator==(const Vector::value_type& e1, const Vector::value_type& e2)
    {
      if (e1.size() != e2.size())
        return false;
      for (unsigned i = 0; i < e1.size(); ++i)
        if (e1(i) != e2(i))
          return false;
      return true;
    }

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
      BIND(PLUS, operator+,
           rVector (Vector::*)());
      BIND(PLUS, operator+,
           rVector (Vector::*)(const rObject&) const);
      /* Something is wrong with the handling of these two overloads.
       * so use a disambiguator.
      BIND(PLUS, operator+,
           rVector (Vector::*)(const rVector&));
      BIND(PLUS, operator+,
           rVector (Vector::*)(ufloat));
           */
      BIND(MINUS, operator-,
           rVector (Vector::*)());
      BIND(MINUS, operator-,
           rVector (Vector::*)(const rObject&) const);
      /*
      BIND(MINUS, operator-, rVector (Vector::*)(const rVector&));
      BIND(MINUS, operator-, rVector (Vector::*)(ufloat));
      */
      BIND(STAR, operator*, rVector (Vector::*)(const rObject&) const);
      BIND(SLASH, operator/, rVector (Vector::*)(const rObject&) const);

      BIND(EQ_EQ, operator==, bool (self_type::*)(const rObject&) const);

      BIND(LT, operator<);
      BIND(SBL_SBR, operator[]);
      BIND(SBL_SBR_EQ, set);
      BIND(asString, as_string);
      BIND(combAdd);
      BIND(combDiv);
      BIND(combMul);
      BIND(combSub);
      BIND(norm);
      BIND(resize);
      BIND(scalarGE);
      BIND(scalarLE);
      BIND(selfCombAdd);
      BIND(selfCombDiv);
      BIND(selfCombIndexes);
      BIND(selfCombMul);
      BIND(selfCombSub);
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
    }                                                   \
                                                        \
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
    OP(Sub, -)
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

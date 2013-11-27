/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/object/global.hh>
#include <urbi/object/vector.hh>
#include <urbi/object/matrix.hh>
#include <kernel/uvalue-cast.hh>
#include <urbi/uvalue.hh>
#include <boost/numeric/ublas/lu.hpp> // boost::numeric::ublas::row

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
      : value_()
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
      BIND(asList, as_list);
      BIND(combAdd);
      BIND(combDiv);
      BIND(combMul);
      BIND(combSub);
      BIND(distance);
      BINDG(norm);
      BIND(resize);
      BIND(scalarGE);
      BIND(scalarLE);
      BIND(set, fromList);
      BINDG(size);
      BIND(sum);
      BIND(trueIndexes);
      BIND(serialize);
      BIND(zip);
      BIND(uvalueSerialize);
      bind(libport::Symbol("scalar" "EQ"), &Vector::scalarEQ);
      bind(libport::Symbol("scalar" "GT"), &Vector::scalarGT);
      bind(libport::Symbol("scalar" "LT"), &Vector::scalarLT);
      slot_set_value(SYMBOL(init), new Primitive(&init));
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

    std::vector<ufloat>
    Vector::as_list() const
    {
      return std::vector<ufloat>(value_.begin(), value_.end());
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

    Vector::value_type
    Vector::zip(const std::vector<rVector>& others) const
    {
      // Make it homogeneous
      std::vector<const value_type*> vals;
      std::vector<unsigned> sizes;
      vals.resize(others.size()+1);
      sizes.resize(others.size()+1);
      vals[0] = &value_;
      sizes[0] = value_.size();
      unsigned total = value_.size();
      for (unsigned i=0; i<others.size(); ++i)
      {
        vals[i+1] = &others[i]->value_;
        sizes[i+1] = vals[i+1]->size();
        total += vals[i+1]->size();
      }
      value_type result(total);
      unsigned s = vals.size();
      unsigned p = 0; // result position
      unsigned r = 0; // source index
      while (p < total)
      {
        for (unsigned i=0; i<s; ++i)
          if (sizes[i] > r)
            result[p++] = (*(vals[i]))[r];
        ++r;
      }
      return result;
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

    std::string
    Vector::serialize(unsigned int wordSize, bool le) const
    {
      std::string res(wordSize * value_.size(), 0);
      for (unsigned i=0; i< value_.size(); ++i)
      {
        long val = value_[i];
        char* start = &res[i * wordSize];
        if (le)
          for (unsigned b=0; b<wordSize; ++b)
            *(start+wordSize - b - 1) = val >> (b*8);
        else
          for (unsigned b=0; b<wordSize; ++b)
            *(start+b) = val >> (b*8);
      }
      return res;
    }

    rObject
    Vector::uvalueSerialize() const
    {
      CAPTURE_GLOBAL(Binary);
      // This is ugly: we cannot go through object-cast as it would give us
      // back the vector. So make the Binary ourselves.
      rObject o(const_cast<Vector*>(this));
      urbi::UValue v = ::uvalue_cast(o);
      rObject res = new object::Object();
      res->proto_add(Binary);
      res->slot_set_value(SYMBOL(keywords),
                    new object::String(v.binary->getMessage()));
      res->slot_set_value(SYMBOL(data),
                    new object::String
                    (std::string(static_cast<char*>(v.binary->common.data),
				 v.binary->common.size)));
      return res;
    }
  }
}

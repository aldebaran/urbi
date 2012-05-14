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
#include <urbi/object/matrix.hh>
#include <boost/numeric/ublas/lu.hpp> // boost::numeric::ublas::row
#include <kernel/uvalue-cast.hh>

#define FORWARD_EXCEPTION(Command)              \
  do {                                          \
    try                                         \
    {                                           \
      Command;                                  \
    }                                           \
    catch (const std::runtime_error& e)         \
    {                                           \
      FRAISE(e.what());                         \
    }                                           \
  } while (false)

namespace urbi
{
  namespace object
  {

    namespace ublas = boost::numeric::ublas;

    ATTRIBUTE_NORETURN
    static inline void
    raise_incompatible_sizes(const matrix_type& m1, const matrix_type& m2,
                             const std::string& hint = "")
    {
      FRAISE("incompatible sizes: %sx%s, %sx%s%s%s",
             m1.size1(), m1.size2(), m2.size1(), m2.size2(),
             hint.empty() ? "" : ": ",
             hint);
    }

    ATTRIBUTE_NORETURN
    static inline void
    raise_incompatible_sizes(const matrix_type& m, const vector_type& v,
                             const std::string& hint = "")
    {
      FRAISE("incompatible sizes: %sx%s, %s%s%s",
             m.size1(), m.size2(), v.size(),
             hint.empty() ? "" : ": ",
             hint);
    }

#define CHECK_SIZE(Matrix1, Matrix2, Assertion)         \
    do {                                                \
      if (!(Assertion))                                 \
        raise_incompatible_sizes((Matrix1), (Matrix2)); \
    } while (0);


    static inline
    void
    check_equal_size(const matrix_type& m1, const matrix_type& m2)
    {
      CHECK_SIZE(m1, m2, m1.size1() == m2.size1() && m1.size2() == m2.size2());
    }

    static inline
    void
    check_equal_size2(const matrix_type& m1, const matrix_type& m2)
    {
      CHECK_SIZE(m1, m2, m1.size2() == m2.size2());
    }

    static inline
    void
    check_equal_size1(const matrix_type& m, const vector_type& v)
    {
      CHECK_SIZE(m, v, m.size1() == v.size());
    }

    static inline
    void
    check_equal_size2(const matrix_type& m, const vector_type& v)
    {
      CHECK_SIZE(m, v, m.size2() == v.size());
    }

    /*---------.
    | Matrix.  |
    `---------*/

    Matrix::Matrix()
      : value_()
    {
      proto_add(proto);
    }

    Matrix::Matrix(size_t i, size_t j)
      : value_(i, j)
    {
      proto_add(proto);
    }

    Matrix::Matrix(value_type v)
      : value_(v)
    {
      proto_add(proto);
    }

    Matrix::Matrix(const Matrix& model)
      : CxxObject()
      , super_comparable_type()
      , value_(model.value_)
    {
      proto_add(proto);
    }

    Matrix::Matrix(const rMatrix& model)
      : CxxObject()
      , value_(model->value_)
    {
      proto_add(proto);
    }

    Matrix::Matrix(const objects_type& model)
      : CxxObject()
      , value_()
    {
      proto_add(proto);
      fromList(model);
    }

    rMatrix
    Matrix::init(const objects_type& args)
    {
      rMatrix self = args[0]->as<Matrix>();
      if (!self)
        runner::raise_type_error(args[1], Matrix::proto);

      if (args.size() == 1)
        runner::raise_arity_error(0, 1);
      else if (args.size() == 2)
      {
        // Lists of non-floats.
        if (rList l = args[1]->as<List>())
        {
          const objects_type& list = l->value_get();
          if (list.empty() || !list[0]->as<Float>())
          {
            self->fromList(list);
            return self;
          }
          // Process list of floats such as [1, 2] in the fallback,
          // i.e., as [[1, 2]] so that we support "init([1], [2])" but
          // also "init([1, 2])".
        }
        else if (rMatrix v = args[1]->as<Matrix>())
        {
          self->value_ = v->value_;
          return self;
        }
      }
      else if (args.size() == 3
               && args[1]->as<Float>()
               && args[2]->as<Float>())
      {
        self->value_ = create_zeros(self,
                                    from_urbi<unsigned>(args[1]),
                                    from_urbi<unsigned>(args[2]));
        return self;
      }

      objects_type effective_args(args.begin() + 1, args.end());
      return self->fromList(effective_args);
    }

    Matrix::value_type
    Matrix::create_zeros(rObject, size_t size1, size_t size2)
    {
      return ublas::zero_matrix<ufloat>(size1, size2);
    }

    Matrix::value_type
    Matrix::create_identity(rObject, size_t size)
    {
      return ublas::identity_matrix<ufloat>(size);
    }

    Matrix::value_type
    Matrix::create_scalars(rObject, size_t size1, size_t size2, ufloat v)
    {
      return ublas::scalar_matrix<ufloat>(size1, size2, v);
    }

    Matrix::value_type
    Matrix::create_ones(rObject self, size_t size1, size_t size2)
    {
      return create_scalars(self, size1, size2, 1.0);
    }

    Matrix::value_type
    Matrix::transpose() const
    {
      return trans(value_);
    }

    Matrix::value_type
    Matrix::inverse() const
    {
      FORWARD_EXCEPTION(return ublas::inverse(value_));
    }

    Matrix::value_type
    Matrix::dot_times(const value_type& m) const
    {
      value_type res(size1(), size2());
      for (size_t i = 0; i < size1(); ++i)
        for (size_t j = 0; j < size2(); ++j)
          res(i, j) = value_(i, j) * m(i, j);
      return res;
    }

    Matrix*
    Matrix::fromList(const objects_type& args)
    {
      size_type width;
      // For the first iteration, register and the matrix width and
      // resize, for following iterations make sure the sizes are
      // correct.
#define CHECK_WIDTH(Width)                                              \
      do {                                                              \
        size_t _width = Width;                                          \
        if (!i)                                                         \
        {                                                               \
          width = _width;                                               \
          value_.resize(height, width, false);                          \
        }                                                               \
        else if (width != _width)                                       \
          FRAISE("expecting rows of size %d, got size %d for row %d",   \
                 width, _width, i + 1);                                 \
      } while (false)

      const size_type height = args.size();
      for (unsigned i = 0; i < height; ++i)
      {
        if (rList l = args[i]->as<List>())
        {
          CHECK_WIDTH(l->size());
          unsigned j = 0;
          foreach (const rObject& o, l->value_get())
          {
            if (rFloat f = o->as<Float>())
              value_(i, j) = f->value_get();
            else
              runner::raise_type_error(args[i], Float::proto);
            ++j;
          }
        }
        else if (rVector l = args[i]->as<Vector>())
        {
          CHECK_WIDTH(l->size());
          unsigned j = 0;
          foreach (ufloat v, l->value_get())
            value_(i, j++) = v;
        }
        else
          runner::raise_type_error(args[i], List::proto);
      }
      return this;
#undef CHECK_WIDTH
    }


// FIXME: We should be able to return value_type, but then, it is
// bind_variadic that does not manage to bind minus and so forth.
// In that case, get rid of Conversion.
#define OP(Op, Res, Conversion, Name, Sym)                      \
    Res                                                         \
    Matrix::Name(const objects_type& args)                      \
    {                                                           \
      rMatrix self = args[0]->as<Matrix>();                     \
      check_arg_count(args, 1);                                 \
      if (rFloat f = args[1]->as<Float>())                      \
        return Conversion(self->operator Op(f->value_get()));   \
      else if (rMatrix m = args[1]->as<Matrix>())               \
        return Conversion(self->operator Op(m->value_get()));   \
      runner::raise_argument_type_error                         \
        (1, args[1], Matrix::proto, to_urbi(SYMBOL(Sym)));      \
    }

    OP(+,  Matrix*, new Matrix, plus,         PLUS)
    OP(-,  Matrix*, new Matrix, minus,        MINUS)
    OP(/,  Matrix*, new Matrix, div,          SLASH)
    OP(*,  Matrix*, new Matrix, times,        STAR)

    OP(+=, Matrix*,           , plus_assign,  PLUS_EQ)
    OP(-=, Matrix*,           , minus_assign, MINUS_EQ)
    OP(/=, Matrix*,           , div_assign,   SLASH_EQ)
    OP(*=, Matrix*,           , times_assign, STAR_EQ)

#undef OP

    /*------------------------------------------------------.
    | Arithmetic and in-place arithmetic between matrices.  |
    `------------------------------------------------------*/

#define OP(Op)                                          \
    Matrix::value_type                                  \
    Matrix::operator Op(const value_type& m) const      \
    {                                                   \
      check_equal_size(value_, m);                      \
      value_type res(value_);                           \
      res Op##= m;                                      \
      return res;                                       \
    }                                                   \
                                                        \
    Matrix*                                             \
    Matrix::operator Op##=(const value_type& m)         \
    {                                                   \
      check_equal_size(value_, m);                      \
      value_ Op##= m;                                   \
      return this;                                      \
    }

    OP(+)
    OP(-)
#undef OP

    Matrix::value_type
    Matrix::operator /(const value_type& rhs) const
    {
      FORWARD_EXCEPTION(return prod(value_, ublas::inverse(rhs)));
    }

    Matrix*
    Matrix::operator /=(const value_type& rhs)
    {
      FORWARD_EXCEPTION(value_ = prod(value_, ublas::inverse(rhs)));
      return this;
    }

    Matrix::value_type
    Matrix::operator *(const value_type& rhs) const
    {
      CHECK_SIZE(value_, rhs, value_.size2() == rhs.size1());
      return prod(value_, rhs);
    }

    Matrix*
    Matrix::operator *=(const value_type& rhs)
    {
      value_ = *this * rhs;
      return this;
    }


    /*--------------------------.
    | Arithmetic between rows.  |
    `--------------------------*/

#define OP(Name, Op)                            \
    Matrix::value_type                          \
    Matrix::Name(const vector_type& rhs) const  \
    {                                           \
      check_equal_size1(value_, rhs);           \
      const size_t height = size1();            \
      const size_t width = size2();             \
      value_type res = value_;                  \
      for (unsigned i = 0; i < height; ++i)     \
        for (unsigned j = 0; j < width; ++j)    \
          res(i, j) Op ## = rhs(i);             \
      return res;                               \
    }

    OP(rowAdd, +)
    OP(rowSub, -)
    OP(rowMul, *)
    OP(rowDiv, /)
#undef OP


    /*--------------------------.
    | Arithmetic with scalars.  |
    `--------------------------*/

#define OP(Op)                                  \
    Matrix::value_type                          \
    Matrix::operator Op(ufloat s) const         \
    {                                           \
      value_type res(value_);                   \
      res Op##= s;                              \
      return res;                               \
    }                                           \
                                                \
    Matrix*                                     \
    Matrix::operator Op##=(ufloat s)            \
    {                                           \
      value_ Op##= s;                           \
      return this;                              \
    }

    OP(*)
    OP(/)

#undef OP

#define OP(Op)                                                  \
    Matrix::value_type                                          \
    Matrix::operator Op(ufloat s) const                         \
    {                                                           \
      value_type copy(value_);                                  \
      value_type ones = create_scalars(0, size1(), size2(), s); \
      value_type res = copy Op ones;                            \
      return res;                                               \
    }                                                           \
                                                                \
    Matrix*                                                     \
    Matrix::operator Op##=(ufloat s)                            \
    {                                                           \
      value_ = operator Op(s);                                  \
      return this;                                              \
    }

    OP(+)
    OP(-)
#undef OP

    URBI_CXX_OBJECT_INIT(Matrix)
      : value_()
    {
      BIND_VARIADIC(MINUS, minus);
      BIND_VARIADIC(MINUS_EQ, minus_assign);
      BIND_VARIADIC(PLUS, plus);
      BIND_VARIADIC(PLUS_EQ, plus_assign);
      BIND_VARIADIC(SLASH, div);
      BIND_VARIADIC(SLASH_EQ, div_assign);
      BIND_VARIADIC(STAR, times);
      BIND_VARIADIC(STAR_EQ, times_assign);

      //BIND(dot_times, dotWiseMult);
      //BIND(solve);
      BIND(EQ_EQ, operator==, bool, (const rObject&) const);
      BIND(SBL_SBR, operator());
      BIND(SBL_SBR_EQ, set);
      BIND(appendRow);
      BIND(asPrintable);
      BIND(asString);
      BIND(asTopLevelPrintable);
      BIND(column);
      BIND(createIdentity, create_identity);
      BIND(createOnes, create_ones);
      BIND(createScalars, create_scalars);
      BIND(createZeros, create_zeros);
      BIND(distanceMatrix);
      BIND(inverse);
      BIND(resize);
      BIND(row);
      BIND(rowAdd);
      BIND(rowDiv);
      BIND(rowMul);
      BIND(rowNorm);
      BIND(rowSub);
      BIND(set, fromList);
      BIND(setRow);
      BINDG(size, size, rObject, () const);
      BIND(transpose);
      BIND(uvalueSerialize);
      slot_set_value(SYMBOL(init), new Primitive(&init));
    }

    std::string
    Matrix::asString() const
    {
      return make_string('<', '>', "<", ">");
    }

    std::string
    Matrix::asPrintable() const
    {
      return make_string('[', ']', "Matrix([", "])");
    }

    std::string
    Matrix::make_string(char col_lsep, char col_rsep,
                        const std::string row_lsep,
                        const std::string row_rsep) const
    {
      const size_t height = size1();
      const size_t width = size2();

      std::ostringstream s;
      s << row_lsep;
      for (unsigned i = 0; i < height; ++i)
      {
        if (i)
          s << ", ";
        s << col_lsep;
        for (unsigned j = 0; j < width; ++j)
        {
          if (j)
            s << ", ";
          s << value_(i, j);
        }
        s << col_rsep;
      }
      s << row_rsep;
      return s.str();
    }


    std::string
    Matrix::asTopLevelPrintable() const
    {
      const size_t height = size1();
      const size_t width = size2();

      std::ostringstream s;
      s << "Matrix([";

      for (unsigned i = 0; i < height; ++i)
      {
        if (i)
          s << ',';
        s << std::endl;
        s << "  [";
        for (unsigned j = 0; j < width; ++j)
        {
          if (j)
            s << ", ";
          s << value_(i, j);
        }
        s << "]";
      }

      s << "])";

      return s.str();
    }

    ufloat
    Matrix::operator()(int i, int j) const
    {
      return get(i, j);
    }

    ufloat
    Matrix::set(int i, int j, ufloat v)
    {
      return value_(index1(i), index2(j)) = v;
    }

    ufloat
    Matrix::get(int i, int j) const
    {
      return value_(index1(i), index2(j));
    }

    rObject
    Matrix::size() const
    {
      CAPTURE_GLOBAL(Pair);
      return Pair->call(SYMBOL(new), new Float(size1()), new Float(size2()));
    }

    Matrix::vector_type
    Matrix::row(int i) const
    {
      return boost::numeric::ublas::row(value_, index1(i));
    }

    Matrix::vector_type
    Matrix::column(int i) const
    {
      return boost::numeric::ublas::column(value_, index2(i));
    }

    Matrix::value_type
    Matrix::distanceMatrix(const value_type& b) const
    {
      check_equal_size2(value_, b);
      const size_t height = size1();
      const size_t width = size2();
      const size_t height2 = b.size1();
      value_type res(height, height2);
      for (unsigned p1 = 0; p1 < height; ++p1)
        for (unsigned p2 = 0; p2 < height2; ++p2)
        {
          ufloat v = 0;
          for (unsigned i=0; i < width; ++i)
          {
            ufloat t = value_(p1, i) - b(p2, i);
            v += t*t;
          }
          res(p1, p2) = sqrt(v);
        }
      return res;
    }

    Matrix::vector_type
    Matrix::rowNorm() const
    {
      // FIXME: there might be something in Boost to do that.
      const size_t height = size1();
      const size_t width = size2();
      vector_type res(height);
      for (unsigned p1 = 0; p1<height; ++p1)
      {
        ufloat v = 0;
        for (unsigned i=0; i<width; ++i)
        {
          ufloat t = value_(p1, i);
          v += t*t;
        }
        res[p1] = sqrt(v);
      }
      return res;
    }

    Matrix*
    Matrix::setRow(int r, const vector_type& v)
    {
      check_equal_size2(value_, v);
      int j = index1(r);
      for (unsigned i = 0; i< v.size(); ++i)
        value_(j, i) = v(i);
      return this;
    }

    Matrix*
    Matrix::appendRow(const vector_type& v)
    {
      check_equal_size2(value_, v);
      value_.resize(size1()+1, size2());
      setRow(size1()-1, v);
      return this;
    }

    Matrix*
    Matrix::resize(size_t ns1, size_t ns2)
    {
      size_t s1 = value_.size1();
      size_t s2 = value_.size2();
      value_.resize(ns1, ns2);
      // Boost does not ensure that we have 0 for new members.
      for (size_t i = 0; i < ns1; ++i)
        for (size_t j = i < s1 ? s2 : 0; j < ns2; ++j)
          value_(i, j) = 0;
      return this;
    }

    rObject
    Matrix::uvalueSerialize() const
    {
      CAPTURE_GLOBAL(Binary);
      // This is ugly: we cannot go through object-cast as it would give us
      // back the vector. So make the Binary ourselve.
      // This is also a duplicate of Vector::uvalueSerialize.
      rObject o(const_cast<Matrix*>(this));
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

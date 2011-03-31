/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/object/matrix.hh>
#include <boost/numeric/ublas/lu.hpp>

namespace urbi
{
  namespace object
  {
    Matrix::Matrix()
      : value_()
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

    Matrix::Matrix(const rList& model)
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
        if (rList l = args[1]->as<List>())
        {
          self->fromList(l);
          return self;
        }
        else if (rMatrix v = args[1]->as<Matrix>())
        {
          self->value_ = v->value_;
          return self;
        }
      }
      else if (args.size() == 3)
      {
        self->value_ =
          boost::numeric::ublas::zero_matrix<ufloat>
          (from_urbi<unsigned>(args[1]), from_urbi<unsigned>(args[2]));
        return self;
      }
      objects_type effective_args(args.begin() + 1, args.end());
      return self->fromArgsList(effective_args);
    }

    rMatrix
    Matrix::create_zeros(rObject, int size1, int size2)
    {
      boost::numeric::ublas::zero_matrix<ufloat> res(size1, size2);
      return new Matrix(res);
    }

    rMatrix
    Matrix::create_identity(rObject, int size)
    {
      boost::numeric::ublas::identity_matrix<ufloat> res(size);
      return new Matrix(res);
    }

    rMatrix
    Matrix::create_scalars(rObject, int size1, int size2, ufloat v)
    {
      boost::numeric::ublas::scalar_matrix<ufloat> res(size1, size2, v);
      return new Matrix(res);
    }

    rMatrix
    Matrix::create_ones(rObject self, int size1, int size2)
    {
      return create_scalars(self, size1, size2, 1.0);
    }

    rMatrix
    Matrix::transpose() const
    {
      return new Matrix(trans(value_));
    }

    rMatrix
    Matrix::invert() const
    {
      using namespace boost::numeric::ublas;
      if (value_.size1() != value_.size2())
        FRAISE("expected square matrix, got %dx%d",
               value_.size1(), value_.size2());
      // Create a working copy of the input.
      value_type A(value_);
      value_type inverse(value_.size1(), value_.size2());
      // Create a permutation matrix for the LU-factorization.
      permutation_matrix<size_type> pm(A.size1());
      // Perform LU-factorization.
      if (lu_factorize(A, pm) != 0)
        FRAISE("non-invertible matrix: %s", *this);
      // Create identity matrix of "inverse".
      inverse.assign(identity_matrix<ufloat>(A.size1()));
      // Backsubstitute to get the inverse.
      lu_substitute(A, pm, inverse);
      return new Matrix(inverse);
    }

    rMatrix
    Matrix::dot_times(const rMatrix& m) const
    {
      value_type res(value_.size1(), value_.size2());
      for (unsigned int i = 0; i < value_.size1(); ++i)
        for (unsigned int j = 0; j < value_.size2(); ++j)
          res(i, j) = value_(i, j) * (*m)(i, j);
      return new Matrix(res);
    }

    rMatrix
    Matrix::fromArgsList(const objects_type& args)
    {
      rList first_row = args[0]->as<List>();
      if (! first_row)
        runner::raise_type_error(args[0], List::proto);

      const size_type width = first_row->size();
      const size_type height = args.size();
      value_.resize(height, width, false);
      for (unsigned i = 0; i < height; ++i)
      {
        if (rList l = args[i]->as<List>())
        {
          if (width != l->size())
            FRAISE("expecting rows of size %d, got size %d for row %d",
                   width, l->size(), i + 1);
          unsigned j = 0;
          foreach(const rObject& o, l->value_get())
          {
            if (rFloat f = o->as<Float>())
              value_(i, j) = f->value_get();
            else
              runner::raise_type_error(args[i], Float::proto);
            ++j;
          }
        }
        else
          runner::raise_type_error(args[i], List::proto);
      }
      return this;
    }

    rMatrix
    Matrix::fromList(const rList& model)
    {
      return fromArgsList(model->value_get());
    }

#define OP(Op, Name, Sym)                                       \
    rMatrix                                                     \
    Matrix::Name(const objects_type& args)                      \
    {                                                           \
      rMatrix self = args[0]->as<Matrix>();                     \
      if (args.size() != 2)                                     \
        runner::raise_arity_error(1, args.size() - 1);          \
      if (rFloat f = args[1]->as<Float>())                      \
        return self->operator Op(f);                            \
      else if (rMatrix m = args[1]->as<Matrix>())               \
        return self->operator Op(m);                            \
      runner::raise_argument_type_error                         \
        (1, args[1], Matrix::proto, to_urbi(SYMBOL(Sym)));      \
    }

    OP(+,  plus,         PLUS)
    OP(-,  minus,        MINUS)
    OP(/,  div,          SLASH)
    OP(*,  times,        STAR)
    OP(+=, plus_assign,  PLUS_EQ)
    OP(-=, minus_assign, MINUS_EQ)
    OP(/=, div_assign,   SLASH_EQ)
    OP(*=, times_assign, STAR_EQ)

#undef OP

#define OP(Op)                                                \
    rMatrix                                                   \
    Matrix::operator Op(const rMatrix& m) const               \
    {                                                         \
      value_type copy(value_);                                \
      copy Op##= m->value_;                                   \
      return new Matrix(copy);                                \
    }                                                         \
                                                              \
    rMatrix                                                   \
    Matrix::operator Op##=(const rMatrix& m)                  \
    {                                                         \
      value_ Op##= m->value_;                                 \
      return this;                                            \
    }

    OP(+)
    OP(-)

#undef OP

#define OP(Name, Op)                                            \
    rMatrix                                                     \
    Matrix::Name(const rVector& rhs) const                      \
    {                                                           \
      const size_t height = value_.size1();                     \
      const size_t width = value_.size2();                      \
      rMatrix res =                                             \
        Matrix::create_zeros(0, height, width)->as<Matrix>();   \
      Matrix::value_type& v = res->value_get();                 \
      const Vector::value_type& v2 = rhs->value_get();          \
      for (unsigned p1 = 0; p1 < height; ++p1)                  \
        for (unsigned i = 0; i < width; ++i)                    \
          v(p1, i) = value_(p1, i) Op v2(p1);                   \
      return res;                                               \
    }

    OP(rowAdd, +)
    OP(rowSub, -)
    OP(rowMul, *)
    OP(rowDiv, /)
#undef OP

    rMatrix
    Matrix::operator /(const rMatrix& rhs) const
    {
      rMatrix inverse = rhs->invert();
      value_type copy = prod(value_, inverse->value_);
      return new Matrix(copy);
    }

    rMatrix
    Matrix::operator /=(const rMatrix& rhs)
    {
      rMatrix inverse = rhs->invert();
      value_type res = prod(value_, inverse->value_);
      value_ = res;
      return this;
    }

#define OP(Op, Type, Fun)                                       \
    rMatrix                                                     \
    Matrix::operator Op(const r##Type& rhs) const               \
    {                                                           \
      value_type copy = Fun(value_, rhs->value_);               \
      return new Matrix(copy);                                  \
    }                                                           \
                                                                \
    rMatrix                                                     \
    Matrix::operator Op##=(const r##Type& rhs)                  \
    {                                                           \
      value_type res = Fun(value_, rhs->value_);                \
      value_ = res;                                             \
      return this;                                              \
    }

    OP(*, Matrix, prod)
    //OP(*, Vector, prod)

#undef OP

#define OP(Op)                                                 \
    rMatrix                                                    \
    Matrix::operator Op(const rFloat& s) const                 \
    {                                                          \
      value_type copy(value_);                                 \
      copy Op##= s->value_get();                               \
      return new Matrix(copy);                                 \
    }                                                          \
                                                               \
    rMatrix                                                    \
    Matrix::operator Op##=(const rFloat& s)                    \
    {                                                          \
      value_ Op##= s->value_get();                             \
      return this;                                             \
    }

    OP(*)
    OP(/)

#undef OP

#define OP(Op)                                                  \
    rMatrix                                                     \
    Matrix::operator Op(const rFloat& s) const                  \
    {                                                           \
      value_type copy(value_);                                  \
      value_type ones =                                         \
        boost::numeric::ublas::scalar_matrix<ufloat>            \
        (value_.size1(), value_.size2(), s->value_get());       \
      value_type res = copy Op ones;                            \
      return new Matrix(res);                                   \
    }                                                           \
                                                                \
    rMatrix                                                     \
    Matrix::operator Op##=(const rFloat& s)                     \
    {                                                           \
      rMatrix res = static_cast<Matrix*>(operator Op(s).get()); \
      value_ = res->value_;                                     \
      return this;                                              \
    }

    OP(+)
    OP(-)

#undef OP

    URBI_CXX_OBJECT_INIT(Matrix)
    {
#define DECLARE(Name, Cxx)                      \
      bind_variadic(SYMBOL(Name), &Matrix::Cxx)

      DECLARE(STAR, times);
      DECLARE(PLUS, plus);
      DECLARE(MINUS, minus);
      DECLARE(SLASH, div);
      DECLARE(STAR_EQ, times_assign);
      DECLARE(PLUS_EQ, plus_assign);
      DECLARE(MINUS_EQ, minus_assign);
      DECLARE(SLASH_EQ, div_assign);

#undef DECLARE

#define DECLARE(Name, Cxx)                                      \
    bind(libport::Symbol("" #Name), &Matrix::Cxx)

      DECLARE(row, row);
      DECLARE(column, column);
      DECLARE(distanceMatrix, distanceMatrix);
      DECLARE(rowNorm, rowNorm);
      DECLARE(setRow, setRow);
      DECLARE(appendRow, appendRow);
      DECLARE(rowAdd, rowAdd);
      DECLARE(rowSub, rowSub);
      DECLARE(rowMul, rowMul);
      DECLARE(rowDiv, rowDiv);
      DECLARE(resize, resize);
      DECLARE(createIdentity, create_identity);
      DECLARE(createZeros, create_zeros);
      DECLARE(createScalars, create_scalars);
      DECLARE(createOnes, create_ones);
      bind(libport::Symbol( "[]" ), &Matrix::operator());
      //DECLARE(SBL_SBR, operator());
      bind(libport::Symbol( "[]=" ), &Matrix::set);
      //DECLARE(SBL_SBR_EQ, set);
      DECLARE(set, set);
      DECLARE(asString, asString);
      DECLARE(asPrintable, asPrintable);
      DECLARE(asToplevelPrintable, asToplevelPrintable);
      //DECLARE(solve, solve);
      DECLARE(invert, invert);
      DECLARE(transpose, transpose);
      DECLARE(distanceToMatrix, distanceToMatrix);
      DECLARE(get, get);
      //DECLARE(dot_times, dotWiseMult);

#undef DECLARE

      bind(SYMBOL(size), static_cast<rObject (Matrix::*)() const>(&Matrix::size));
      bind(SYMBOL(set), static_cast<rMatrix (Matrix::*)(const rList&)>(&Matrix::fromList));
      slot_set(SYMBOL(init), new Primitive(&init));
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
      const unsigned int height = value_.size1();
      const unsigned int width = value_.size2();

      std::ostringstream s;
      s << row_lsep;
      for (unsigned i = 0; i < height; ++i)
      {
        s << col_lsep;
        for (unsigned j = 0; j < width; ++j)
        {
          s << value_(i, j);
          if (j != width - 1)
            s << ", ";
        }
        s << col_rsep;
        if (i != height - 1)
          s << ", ";
      }
      s << row_rsep;
      return s.str();
    }


    std::string
    Matrix::asToplevelPrintable() const
    {
      const unsigned int height = value_.size1();
      const unsigned int width = value_.size2();

      std::ostringstream s;
      s << "Matrix([" << std::endl;

      for (unsigned i = 0; i < height; ++i)
      {
        s << "  [";
        for (unsigned j = 0; j < width; ++j)
        {
          s << libport::format("%e", value_(i, j));
          if (j != width - 1)
            s << ", ";
        }
        s << "]";
        if (i != height - 1)
          s << ", ";
        s << std::endl;
      }

      s << "])";

      return s.str();
    }

    ufloat
    Matrix::operator()(int i, int j) const
    {
      return value_(index1(i), index2(j));
    }

    rMatrix
    Matrix::set(int i, int j, ufloat v)
    {
      value_(index1(i), index2(j)) = v;
      return this;
    }

    ufloat
    Matrix::get(int i, int j)
    {
      return value_(index1(i), index2(j));
    }

    rObject
    Matrix::size() const
    {
      CAPTURE_GLOBAL(Pair);
      return Pair->call(SYMBOL(new),
                        new Float(value_.size1()),
                        new Float(value_.size2()));
    }

    rVector
    Matrix::row(int i) const
    {
      return new Vector(boost::numeric::ublas::row(value_, index1(i)));
    }

    rVector
    Matrix::column(int i) const
    {
      return new Vector(boost::numeric::ublas::column(value_, index2(i)));
    }

    rVector
    Matrix::distanceMatrix() const
    {
      const unsigned int height = value_.size1();
      const unsigned int width = value_.size2();
      rVector res(new Vector(height * (height-1) / 2));
      unsigned idx = 0;
      for (unsigned p1 = 0; p1<height; ++p1)
        for (unsigned p2 = p1+1; p2 < height; ++p2)
        {
          ufloat v = 0;
          for (unsigned i=0; i<width; ++i)
          {
            ufloat t = value_(p1, i) - value_(p2, i);
            v += t*t;
          }
          res->value_get()[idx++] = sqrt(v);
        }
      return res;
    }

    rVector
    Matrix::distanceToMatrix(rMatrix b) const
    {
      Matrix::value_type& vb = b->value_get();
      const unsigned int height = value_.size1();
      const unsigned int width = value_.size2();
      const unsigned int height2 = vb.size1();
      if (width != vb.size2())
        throw std::runtime_error("Incompatible matrix sizes.");
      rVector res(new Vector(height * height2));
      unsigned idx = 0;
      for (unsigned p1 = 0; p1<height; ++p1)
        for (unsigned p2 = 0; p2 < height2; ++p2)
        {
          ufloat v = 0;
          for (unsigned i=0; i<width; ++i)
          {
            ufloat t = value_(p1, i) - vb(p2, i);
            v += t*t;
          }
          res->value_get()[idx++] = sqrt(v);
        }
      return res;
    }

    rVector
    Matrix::rowNorm() const
    {
      const unsigned int height = value_.size1();
      const unsigned int width = value_.size2();
      rVector res(new Vector(height));
      for (unsigned p1 = 0; p1<height; ++p1)
      {
        ufloat v = 0;
        for (unsigned i=0; i<width; ++i)
        {
          ufloat t = value_(p1, i);
          v += t*t;
        }
        res->value_get()[p1] = sqrt(v);
      }
      return res;
    }

    rMatrix
    Matrix::setRow(int r, rVector v)
    {
      int j = index1(r);
      index2(v->size()-1); // Check size
      Vector::value_type& val = v->value_get();
      for (unsigned i = 0; i< v->size(); ++i)
        value_(j, i) = val(i);
      return this;
    }

    rMatrix
    Matrix::appendRow(rVector v)
    {
      value_.resize(value_.size1()+1, value_.size2());
      setRow(value_.size1()-1, v);
      return this;
    }

    rMatrix
    Matrix::resize(unsigned int i, unsigned int j)
    {
      value_.resize(i, j);
      return this;
    }
  }
}

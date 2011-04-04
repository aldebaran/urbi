/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_MATRIX_HH
# define OBJECT_MATRIX_HH

# include <urbi/object/cxx-object.hh>
# include <urbi/object/fwd.hh>

# include <libport/system-warning-push.hh>
#  include <boost/numeric/ublas/blas.hpp>
#  include <boost/numeric/ublas/matrix.hpp>
#  include <boost/numeric/ublas/vector.hpp>
# include <libport/system-warning-pop.hh>

# include <urbi/object/vector.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Matrix
      : public CxxObject
      , public EqualityComparable<Matrix,
                                  boost::numeric::ublas::matrix<ufloat> >
    {
    public:
      typedef Matrix self_type;
      typedef boost::numeric::ublas::matrix<ufloat> value_type;
      typedef EqualityComparable<self_type, value_type> super_comparable_type;
      typedef value_type::size_type size_type;

      Matrix();
      Matrix(unsigned int s);
      Matrix(value_type v);
      Matrix(const Matrix& v);
      Matrix(const rMatrix& model);
      Matrix(const rList& model);

      static rMatrix init(const objects_type& args);
      rMatrix fromList(const rList& model);

#define OP(Name)                                        \
      static rMatrix Name(const objects_type& args)

      OP(plus);
      OP(minus);
      OP(times);
      OP(div);
      OP(plus_assign);
      OP(minus_assign);
      OP(times_assign);
      OP(div_assign);

#undef OP

#define OP(Op, Type)                                      \
      rMatrix operator Op(const r##Type& m) const;        \
      rMatrix operator Op##=(const r##Type& m)

      OP(+, Matrix);
      OP(-, Matrix);
      OP(*, Matrix);
      OP(/, Matrix);
      //OP(*, Vector);

#undef OP

#define OP(Op)                                          \
      rMatrix operator Op(const rFloat& m) const;       \
      rMatrix operator Op##=(const rFloat& m);

      OP(*);
      OP(/);
      OP(+);
      OP(-);

#undef OP

      // Row by row operation.
      rMatrix rowAdd(const rVector& rhs) const;
      rMatrix rowSub(const rVector& rhs) const;
      rMatrix rowMul(const rVector& rhs) const;
      rMatrix rowDiv(const rVector& rhs) const;
      rVector rowNorm() const;

      static rMatrix create_zeros(rObject, int size1, int size2);
      static rMatrix create_identity(rObject, int size);
      static rMatrix create_scalars(rObject, int size1, int size2, ufloat v);
      static rMatrix create_ones(rObject, int size1, int size2);

      rMatrix transpose() const;
      rMatrix invert() const;
      rMatrix solve(const rVector& vector) const;

      ufloat operator()(int, int) const;

      rObject size() const;
      rMatrix resize(unsigned int i, unsigned int j);
      rMatrix set(int i, int j, ufloat v);
      ufloat get(int i, int j);
      unsigned int index1(int) const;
      unsigned int index2(int) const;

      value_type&       value_get();
      const value_type& value_get() const;
      std::string asString() const;
      std::string asPrintable() const;
      std::string asTopLevelPrintable() const;

      rMatrix dot_times(const rMatrix& m) const;
      rVector row(int i) const;
      rVector column(int i) const;
      // Row-by-row l2 interdistance matrix
      rVector distanceMatrix() const;
      // Row-by-row cross-distance matrix, self-major v-minor
      rVector distanceToMatrix(rMatrix v) const;
      rMatrix setRow(int r, rVector val);
      rMatrix appendRow(rVector vals);
      rObject uvalueSerialize() const;
    private:
      std::string make_string(char col_lsep, char col_rsep,
                              const std::string row_lsep,
                              const std::string row_rsep) const;
      rMatrix fromArgsList(const objects_type& rows);

      value_type value_;
      URBI_CXX_OBJECT(Matrix, CxxObject);
    };

    URBI_SDK_API
    bool
    operator==(const Matrix::value_type& e1, const Matrix::value_type& e2);

  } // namespace object
} // namespace urbi

# include <urbi/object/matrix.hxx>

#endif

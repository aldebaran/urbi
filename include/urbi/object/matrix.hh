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
      typedef Vector::matrix_type matrix_type;
      typedef Vector::vector_type vector_type;
      typedef matrix_type value_type;
      typedef EqualityComparable<self_type, value_type> super_comparable_type;
      typedef value_type::size_type size_type;

      Matrix();
      Matrix(size_t i, size_t j);
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
      rMatrix rowAdd(const vector_type& rhs) const;
      rMatrix rowSub(const vector_type& rhs) const;
      rMatrix rowMul(const vector_type& rhs) const;
      rMatrix rowDiv(const vector_type& rhs) const;
      vector_type rowNorm() const;

      static rMatrix create_zeros(rObject, int size1, int size2);
      static rMatrix create_identity(rObject, int size);
      static rMatrix create_scalars(rObject, int size1, int size2, ufloat v);
      static rMatrix create_ones(rObject, int size1, int size2);

      rMatrix transpose() const;
      rMatrix invert() const;
      rMatrix solve(const vector_type& vector) const;

      ufloat operator()(int, int) const;

      rObject size() const;
      size_t size1() const;
      size_t size2() const;
      rMatrix resize(size_t i, size_t j);
      rMatrix set(int i, int j, ufloat v);
      ufloat get(int i, int j);
      size_t index1(int) const;
      size_t index2(int) const;

      value_type&       value_get();
      const value_type& value_get() const;
      std::string asString() const;
      std::string asPrintable() const;
      std::string asTopLevelPrintable() const;

      rMatrix dot_times(const rMatrix& m) const;
      vector_type row(int i) const;
      vector_type column(int i) const;
      // Row-by-row l2 interdistance matrix
      value_type distanceMatrix() const;
      // Row-by-row cross-distance matrix, self-major v-minor
      value_type distanceToMatrix(const value_type& v) const;
      rMatrix setRow(int r, const vector_type& val);
      rMatrix appendRow(const vector_type& vals);
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


    /*-------------.
    | Conversion.  |
    `-------------*/

    template <>
    struct CxxConvert<Matrix::value_type>
    {
      typedef       Matrix::value_type& target_type;
      typedef const Matrix::value_type& source_type;
      static target_type
      to(const rObject& o)
      {
        type_check<Matrix>(o);
        return o->as<Matrix>()->value_get();
      }

      static rObject
      from(source_type v)
      {
        return new Matrix(v);
      }
    };

  } // namespace object
} // namespace urbi

# include <urbi/object/matrix.hxx>

#endif
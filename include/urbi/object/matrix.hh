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
      , public EqualityComparable<Matrix, matrix_type >
    {
    public:
      typedef Matrix self_type;
      typedef ::urbi::object::matrix_type matrix_type;
      typedef ::urbi::object::vector_type vector_type;
      typedef matrix_type value_type;
      typedef EqualityComparable<self_type, value_type> super_comparable_type;
      typedef value_type::size_type size_type;

      Matrix();
      Matrix(size_t i, size_t j);
      Matrix(value_type v);
      Matrix(const Matrix& v);
      Matrix(const rMatrix& model);
      // Initialize from a List.
      Matrix(const objects_type& model);

      static rMatrix init(const objects_type& args);
      Matrix* fromList(const objects_type& model);

#define OP(Name)                                                \
      static Matrix* Name(const objects_type& args);         \
      static Matrix* Name ## _assign(const objects_type& args)

      OP(plus);
      OP(minus);
      OP(times);
      OP(div);
#undef OP

#define OP(Op)                                                  \
      value_type operator Op(const value_type& m) const;        \
      Matrix* operator Op##=(const value_type& m)

      OP(+);
      OP(-);
      OP(*);
      OP(/);
      //OP(*, Vector);
#undef OP

#define OP(Op)                                  \
      value_type operator Op(ufloat m) const;   \
      Matrix* operator Op##=(ufloat m);

      OP(*);
      OP(/);
      OP(+);
      OP(-);
#undef OP

      // Row by row operation.
      value_type rowAdd(const vector_type& rhs) const;
      value_type rowSub(const vector_type& rhs) const;
      value_type rowMul(const vector_type& rhs) const;
      value_type rowDiv(const vector_type& rhs) const;
      vector_type rowNorm() const;

      static value_type create_zeros(rObject, int size1, int size2);
      static value_type create_identity(rObject, int size);
      static value_type create_scalars(rObject, int size1, int size2, ufloat v);
      static value_type create_ones(rObject, int size1, int size2);

      value_type transpose() const;
      static value_type inverse(const value_type& m);
      value_type inverse() const;
      value_type solve(const vector_type& vector) const;

      ufloat operator()(int, int) const;

      rObject size() const;
      size_t size1() const;
      size_t size2() const;
      Matrix* resize(size_t i, size_t j);
      ufloat set(int i, int j, ufloat v);
      ufloat get(int i, int j) const;
      size_t index1(int) const;
      size_t index2(int) const;

      value_type&       value_get();
      const value_type& value_get() const;
      std::string asString() const;
      std::string asPrintable() const;
      std::string asTopLevelPrintable() const;

      value_type dot_times(const value_type& m) const;
      vector_type row(int i) const;
      vector_type column(int i) const;
      // Row-by-row l2 interdistance matrix
      value_type distanceMatrix() const;
      // Row-by-row cross-distance matrix, self-major v-minor
      value_type distanceToMatrix(const value_type& v) const;
      Matrix* setRow(int r, const vector_type& val);
      Matrix* appendRow(const vector_type& vals);
      rObject uvalueSerialize() const;
    private:
      std::string make_string(char col_lsep, char col_rsep,
                              const std::string row_lsep,
                              const std::string row_rsep) const;
      value_type value_;
      URBI_CXX_OBJECT(Matrix, CxxObject);
    };


    /*-------------.
    | Conversion.  |
    `-------------*/

    template <>
    struct CxxConvert<matrix_type>
    {
      typedef       matrix_type& target_type;
      typedef const matrix_type& source_type;
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

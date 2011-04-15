/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_VECTOR_HH
# define OBJECT_VECTOR_HH

# include <urbi/object/cxx-object.hh>
# include <urbi/object/fwd.hh>

# include <libport/system-warning-push.hh>
#  include <boost/numeric/ublas/blas.hpp>
#  include <boost/numeric/ublas/matrix.hpp>
#  include <boost/numeric/ublas/vector.hpp>
# include <libport/system-warning-pop.hh>

namespace urbi
{
  namespace object
  {

    /*--------------.
    | vector_type.  |
    `--------------*/

    typedef boost::numeric::ublas::vector<ufloat> vector_type;

    URBI_SDK_API
    bool
    operator==(const vector_type& e1, const vector_type& e2);


    /*--------------.
    | matrix_type.  |
    `--------------*/

    typedef boost::numeric::ublas::matrix<ufloat> matrix_type;

    URBI_SDK_API
    bool
    operator==(const matrix_type& e1, const matrix_type& e2);

    URBI_SDK_API
    std::ostream&
    operator<<(std::ostream& o, const matrix_type& v);
  }
}

namespace boost
{
  namespace numeric
  {
    namespace ublas
    {

      URBI_SDK_API
      std::ostream&
      operator<<(std::ostream& o, const ::urbi::object::vector_type& v);

      URBI_SDK_API
      std::ostream&
      operator<<(std::ostream& o, const ::urbi::object::matrix_type& v);

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

    class URBI_SDK_API Vector
      : public CxxObject
      , public EqualityComparable<Vector, vector_type>
    {
      friend class Matrix;

    public:
      typedef Vector self_type;
      typedef ::urbi::object::matrix_type matrix_type;
      typedef ::urbi::object::vector_type vector_type;
      typedef vector_type value_type;
      typedef EqualityComparable<self_type, value_type> super_comparable_type;

      Vector();
      Vector(size_t s);
      Vector(const rVector& v);
      Vector(const value_type& model);
      Vector(const objects_type& model);

      static rVector init(const objects_type& args);
      Vector* fromList(const objects_type& model);

      // Disambiguation operators.
      value_type operator+(const rObject& b) const;
      value_type operator-(const rObject& b) const;
      value_type operator*(const rObject& b) const;
      value_type operator/(const rObject& b) const;

      value_type operator+(const value_type& b) const;
      value_type operator-(const value_type& b) const;
      value_type operator*(const value_type& v) const;
      value_type operator/(const value_type& v) const;

      value_type operator+(ufloat v) const;
      value_type operator-(ufloat v) const;
      value_type operator*(ufloat v) const;
      value_type operator/(ufloat v) const;

      Vector* operator+();
      value_type operator-();
      virtual bool operator<(const Object& rhs) const;
      bool    operator<(const value_type& b) const;
      ufloat  operator[](int) const;

      size_t size() const;
      Vector* resize(size_t s);
      ufloat distance(const value_type& that) const;
      ufloat norm() const;
      ufloat sum() const;
      ufloat set(int i, ufloat v);
      size_t index(int) const;

      virtual std::string as_string() const;

      value_type&       value_get();
      const value_type& value_get() const;

      // Combinatorial operations generating a Matrix of size size*b.size().
      matrix_type combAdd(const value_type& b) const;
      matrix_type combDiv(const value_type& b) const;
      matrix_type combMul(const value_type& b) const;
      matrix_type combSub(const value_type& b) const;

      // Return the indices from the original vector after a selfComb op.
      std::pair<size_t, size_t> selfCombIndexes(size_t v);
      // Scalar field by field comparison operations
      value_type scalarGT(const value_type &b) const;
      value_type scalarGE(const value_type &b) const;
      value_type scalarLT(const value_type &b) const;
      value_type scalarLE(const value_type &b) const;
      value_type scalarEQ(const value_type &b) const;
      // Return the vector of true indices.
      value_type trueIndexes() const;
      rObject uvalueSerialize() const;
    private:
      value_type value_;
      URBI_CXX_OBJECT(Vector, CxxObject);
    };


    /*-------------.
    | Conversion.  |
    `-------------*/

    template <>
    struct CxxConvert<Vector::value_type>
    {
      typedef       Vector::value_type& target_type;
      typedef const Vector::value_type& source_type;
      static target_type
      to(const rObject& o)
      {
        type_check<Vector>(o);
        return o->as<Vector>()->value_get();
      }

      static rObject
      from(source_type v)
      {
        return new Vector(v);
      }
    };

  };
}

# include <urbi/object/vector.hxx>

#endif

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
#  include <boost/numeric/ublas/vector.hpp>
# include <libport/system-warning-pop.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Vector
      : public CxxObject
      , public EqualityComparable<Vector,
                                  boost::numeric::ublas::vector<ufloat> >
    {
      friend class Matrix;

    public:
      typedef Vector self_type;
      typedef boost::numeric::ublas::vector<ufloat> value_type;
      typedef value_type vector_type;
      typedef boost::numeric::ublas::matrix<ufloat> matrix_type;
      typedef EqualityComparable<self_type, value_type> super_comparable_type;

      Vector();
      Vector(size_t s);
      Vector(value_type v);
      Vector(const Vector& v);
      Vector(const rVector& model);
      Vector(const rList& model);

      static rVector init(const objects_type& args);
      rVector fromList(const rList& model);

      // Disambiguation operators.
      rVector operator+(const rObject& b) const;
      rVector operator-(const rObject& b) const;
      rVector operator*(const rObject& b) const;
      rVector operator/(const rObject& b) const;

      rVector operator+(const rVector& b) const;
      rVector operator-(const rVector& b) const;
      rVector operator*(const rVector& v) const;
      rVector operator/(const rVector& v) const;

      rVector operator+(ufloat v) const;
      rVector operator-(ufloat v) const;
      rVector operator*(ufloat v) const;
      rVector operator/(ufloat v) const;

      rVector operator+();
      rVector operator-();
      bool    operator<(const Object& b) const;
      ufloat  operator[](int) const;

      size_t size() const;
      void resize(size_t s);
      ufloat norm() const;
      ufloat sum() const; //a.k.a. L1 norm.
      rVector set(int i, ufloat v);
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
      rVector scalarGT(const rVector &b) const;
      rVector scalarGE(const rVector &b) const;
      rVector scalarLT(const rVector &b) const;
      rVector scalarLE(const rVector &b) const;
      rVector scalarEQ(const rVector &b) const;
      // Return the vector of true indices.
      rVector trueIndexes() const;
      rObject uvalueSerialize() const;
    private:
      value_type value_;
      URBI_CXX_OBJECT(Vector, CxxObject);
    };

    URBI_SDK_API
    bool
    operator==(const Vector::value_type& e1, const Vector::value_type& e2);

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

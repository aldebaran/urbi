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

# include <boost/numeric/ublas/blas.hpp>
# include <boost/numeric/ublas/vector.hpp>

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
      typedef EqualityComparable<self_type, value_type> super_comparable_type;

      Vector();
      Vector(unsigned int s);
      Vector(value_type v);
      Vector(const Vector& v);
      Vector(const rVector& model);
      Vector(const rList& model);

      static rObject init(const objects_type& args);
      rObject fromList(const rList& model);

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

      unsigned int size() const;
      void resize(unsigned int s);
      ufloat norm() const;
      ufloat sum() const; //a.k.a. L1 norm.
      rObject set(int i, ufloat v);
      unsigned int index(int) const;

      virtual std::string as_string() const;

      value_type&       value_get();
      const value_type& value_get() const;

      // Combinatorial operations generating a vector of size size*b.size()
      rVector combAdd(rVector b) const;
      rVector combMul(rVector b) const;
      rVector combDiv(rVector b) const;
      // Combinatorial on oneself, generating a vector of size s(s-1)/2
      rVector selfCombAdd() const;
      rVector selfCombMul() const;
      rVector selfCombDiv() const;
      // Return the indices from the original vector after a selfComb op.
      std::pair<unsigned int, unsigned int> selfCombIndexes(unsigned int v);
      // Scalar field by field comparison operations
      rVector scalarGT(const rVector &b) const;
      rVector scalarGE(const rVector &b) const;
      rVector scalarLT(const rVector &b) const;
      rVector scalarLE(const rVector &b) const;
      rVector scalarEQ(const rVector &b) const;
      // Return the vector of true indices.
      rVector trueIndexes() const;
    private:
      value_type value_;
      URBI_CXX_OBJECT(Vector, CxxObject);
    };
  }
}

# include <urbi/object/vector.hxx>

#endif

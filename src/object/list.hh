/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/**
 ** \file object/list-class.hh
 ** \brief Definition of the URBI object list.
 */

#ifndef OBJECT_LIST_CLASS_HH
# define OBJECT_LIST_CLASS_HH

# include <object/cxx-object.hh>
# include <object/fwd.hh>
# include <urbi/export.hh>

namespace object
{
  class URBI_SDK_API List: public object::CxxObject
  {
  public:
    typedef objects_type value_type;
    typedef value_type::size_type size_type;

    List();
    List(const value_type& value);
    List(const rList& model);
    const value_type& value_get() const;
    value_type& value_get();

    // Urbi method
    /// False iff empty.
    virtual bool as_bool() const;
    rObject back        ();
    rList   clear       ();
    void    each        (const rObject&);
    void    each_pipe   (const rObject&);
    void    each_common (const rObject&, bool);
    void    each_and    (const rObject&);
    bool empty() const;
    rObject front       ();
    /// Also known as pop.
    rObject removeFront ();
    rObject removeBack  ();
    rList   insert      (const rFloat& idx, const rObject& elt);
    rList   insertBack  (const rObject& elt);
    rList   insertFront (const rObject& elt);

    std::string asString() const;

    rList   remove_by_id(const rObject& elt);
    rList   reverse     ();
    rObject set         (const rFloat& nth, const rObject& value);
    rFloat  size        ();
    value_type
            sort        ();
    value_type
            sort        (rObject f);
    rList   tail        ();
    rList   operator+   (const rList& rhs);
    rList   operator+=  (const rList& rhs);
    rList   operator*   (unsigned int times);
    rObject operator[]  (const rFloat& idx);

  private:
    value_type content_;
    /// Check that the function fun is using a valid index, and return it.
    size_type index(const rFloat& idx) const;

    URBI_CXX_OBJECT(List);

  public:
    static bool list_added;

  };

}; // namespace object

# include <object/cxx-object.hxx>

#endif // !OBJECT_LIST_CLASS_HH

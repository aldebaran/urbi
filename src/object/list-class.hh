/**
 ** \file object/list-class.hh
 ** \brief Definition of the URBI object list.
 */

#ifndef OBJECT_LIST_CLASS_HH
# define OBJECT_LIST_CLASS_HH

# include <object/cxx-object.hh>
# include <object/fwd.hh>

namespace object
{
  extern rObject list_class;

  class List: public object::CxxObject
  {
  public:
    typedef objects_type value_type;

    List();
    List(const value_type& value);
    List(rList model);
    const value_type& value_get() const;
    value_type& value_get();

    // Urbi method
    rObject back      ();
    rList   clear     ();
    void    each      (runner::Runner&, rObject);
    rObject front     ();
    rList   pop_back  ();
    rList   pop_front ();
    rList   push_back (rObject elt);
    rList   push_front(rObject elt);
    rList   remove_by_id(rObject elt);
    rFloat  size      ();
    rList   sort      (runner::Runner& r);
    rList   tail      ();
    rList   operator+ (rList rhs);
    rList   operator+=(rList rhs);
    rObject operator[](rFloat idx);

    static const std::string type_name;
    virtual std::string type_name_get() const;

  private:
    value_type content_;

  public:
    static void initialize(CxxObject::Binder<List>& binder);
    static bool list_added;
  };

}; // namespace object

#endif // !OBJECT_LIST_CLASS_HH

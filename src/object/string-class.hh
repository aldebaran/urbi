/**
 ** \file object/string-class.hh
 ** \brief Definition of the URBI object string.
 */

#ifndef OBJECT_STRING_CLASS_HH
# define OBJECT_STRING_CLASS_HH

# include <object/cxx-object.hh>

namespace object
{
  extern rObject string_class;

  class String: public CxxObject
  {
    public:
      typedef libport::Symbol value_type;

      String();
      String(rString model);
      String(const value_type& value);
      const value_type& value_get() const;
      value_type& value_get();

      /// Urbi methods
      rString plus(runner::Runner& r, rObject rhs);
      rObject lt(rString rhs);
      rString fresh ();
      rString set(rString rhs);
      rFloat  size();
      rList   split(rString sep);


      static const std::string type_name;
      virtual std::string type_name_get() const;

    private:
      value_type content_;

    public:
      static void initialize(CxxObject::Binder<String>& binder);
      static bool string_added;
  };

  // Urbi functions
  rString as_string(rObject from);
  rString as_printable(rObject from);

}; // namespace object

# include <object/cxx-object.hxx>

#endif // !OBJECT_STRING_CLASS_HH

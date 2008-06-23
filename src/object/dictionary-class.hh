/**
 ** \file object/dictionary-class.hh
 ** \brief Definition of the URBI object dictionary.
 */

#ifndef OBJECT_DICTIONARY_CLASS_HH
# define OBJECT_DICTIONARY_CLASS_HH

# include <libport/hash.hh>
# include <object/fwd.hh>
# include <object/cxx-object.hh>

namespace object
{
  class Dictionary: public CxxObject
  {
  public:
    typedef libport::hash_map<libport::Symbol, rObject> value_type;

    Dictionary();
    Dictionary(rDictionary model);
    Dictionary(const value_type& value);
    const value_type& value_get() const;
    value_type& value_get();

    /// Urbi methods
    rDictionary set(rString key, rObject value);
    rObject get(rString key);
    rObject has(rString key);
    rDictionary clear();
    rList keys();

    static const std::string type_name;
    virtual std::string type_name_get() const;

  private:
    value_type content_;

  public:
    static void initialize(CxxObject::Binder<Dictionary>& binder);
    static bool dictionary_added;
  };
}; // namespace object

#endif // !OBJECT_DICTIONARY_CLASS_HH

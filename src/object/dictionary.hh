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
  extern rObject dictionary_class;

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
    rDictionary clear();
    bool empty();
    rDictionary erase(libport::Symbol key);
    rObject get(libport::Symbol key);
    bool has(libport::Symbol key);
    rList keys();
    rDictionary set(libport::Symbol key, rObject value);

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

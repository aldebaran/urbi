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
  class URBI_SDK_API Dictionary: public CxxObject
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
    bool empty() const;
    /// False iff empty.
    virtual bool as_bool() const;
    rDictionary erase(libport::Symbol key);
    rObject get(libport::Symbol key);
    bool has(libport::Symbol key) const;
    rList keys();
    rDictionary set(libport::Symbol key, rObject value);

  private:
    value_type content_;

  URBI_CXX_OBJECT(Dictionary);
  };
}; // namespace object

#endif // !OBJECT_DICTIONARY_CLASS_HH

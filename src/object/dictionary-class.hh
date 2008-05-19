/**
 ** \file object/dictionary-class.hh
 ** \brief Definition of the URBI object dictionary.
 */

#ifndef OBJECT_DICTIONARY_CLASS_HH
# define OBJECT_DICTIONARY_CLASS_HH

# include <libport/hash.hh>
# include "object/fwd.hh"

namespace object
{
  extern rObject dictionary_class;

  namespace
  {
    typedef libport::hash_map<libport::Symbol, rObject> Dict;
  }
  /// Initialize the Dictionary class.
  void dictionary_class_initialize ();
  std::ostream& operator << (std::ostream& where,
                             const Dict& what);

}; // namespace object

#endif // !OBJECT_DICTIONARY_CLASS_HH

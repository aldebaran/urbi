/**
 ** \file parser/metavar-map.hxx
 ** \brief Implementation of parse::MetavarMap.
 */

#ifndef PARSER_METAVAR_MAP_HXX
# define PARSER_METAVAR_MAP_HXX

# include <sstream>
# include "parser/metavar-map.hh"

namespace parser
{

  template <typename Data>
  MetavarMap<Data>::MetavarMap (const std::string& name)
    : name_ (name), map_ ()
  {
  }

  template <typename Data>
  MetavarMap<Data>::~MetavarMap ()
  {
    // All the values must have been used.
    passert (map_, map_.empty ());
  }

  template <typename Data>
  std::ostream&
  MetavarMap<Data>::dump (std::ostream& o) const
  {
    return o
      << name_ << " map:"
      << libport::incendl << map_ << libport::decendl;
  }

  template <typename Data>
  std::string
  MetavarMap<Data>::append_ (unsigned& count, Data* data)
  {
    map_[count] = data;
    std::string s = "_" + name_ + " (" +
      boost::lexical_cast<std::string> (count++) + ")";
    return s;
  }

  template <typename Data>
  bool
  MetavarMap<Data>::must_be_unique_ (Data*) const
  {
    return true;
  }

  template <typename Data>
  Data*
  MetavarMap<Data>::take_ (unsigned key) throw (std::range_error)
  {
    return map_.take (key);
  }


  /*--------------------------.
  | Free standing functions.  |
  `--------------------------*/

  template <typename Data>
  std::ostream&
  operator<< (std::ostream& o, const MetavarMap<Data>& t)
  {
    return t.dump (o);
  }

}

#endif // !PARSER_METAVAR_MAP_HXX

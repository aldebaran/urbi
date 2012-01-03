/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file parser/metavar-map.hxx
 ** \brief Implementation of parse::MetavarMap.
 */

#ifndef PARSER_METAVAR_MAP_HXX
# define PARSER_METAVAR_MAP_HXX

# include <sstream>
# include <libport/format.hh>
# include <parser/metavar-map.hh>

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
    passert (map_, empty_ ());
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
  MetavarMap<Data>::append_ (unsigned& count, Data data)
  {
    map_[count] = data;
    return libport::format("_%s (%s)", name_, count++);
  }

  template <typename Data>
  Data
  MetavarMap<Data>::take_ (unsigned key) throw (std::range_error)
  {
    passert("Missing meta-variable in " << name_ << " map: " << key,
            libport::mhas(map_, key));
    return map_.take (key);
  }


  template <typename Data>
  void
  MetavarMap<Data>::insert_(MetavarMap<Data>& other)
  {
    return map_.insert(other.map_);
  }


  template <typename Data>
  bool
  MetavarMap<Data>::must_be_unique_ (Data) const
  {
    return true;
  }

  template <typename Data>
  bool
  MetavarMap<Data>::empty_ () const
  {
    return map_.empty();
  }

  template <typename Data>
  void
  MetavarMap<Data>::clear_()
  {
    map_.clear();
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

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
 ** \file parser/metavar-map.hh
 ** \brief Declaration of parser::MetavarMap.
 */

#ifndef PARSER_METAVAR_MAP_HH
# define PARSER_METAVAR_MAP_HH

# include <string>
# include <libport/map.hh>

namespace parser
{

  /// A generic map of metavariables.
  template <typename Data>
  class MetavarMap
  {
  public:
    /// Build a map of metavariables of kind \a name.
    MetavarMap (const std::string& name);
    virtual ~MetavarMap ();

    /// User friendly dump of the table.
    /// Expects the elements of the map to support <<.
    std::ostream& dump (std::ostream& o) const;

  protected:
    /// Append a metavariable to the map.
    virtual std::string append_ (unsigned& key, Data data);

    /// Extract a metavariable.
    virtual Data take_ (unsigned key) throw (std::range_error);

    /// Empty in another map.
    void insert_(MetavarMap<Data>& other);

    /// Whether this value must be unique.
    /// Used to catch multiple uses of a unique pointer.
    virtual bool must_be_unique_ (Data) const;

    /// Check the tables are empty.
    /// which is abnormal.
    bool empty_() const;

    /// Clear the tables. This must be used only while recovering from
    /// abnormal conditions.
    void clear_();

    /// Name of the kind of variable.
    const std::string name_;

    /// Metavariables.
    typedef libport::map<unsigned, Data> map_type;
    map_type map_;
  };



  /*--------------------------.
  | Free standing functions.  |
  `--------------------------*/

  template <typename Data>
  std::ostream&
  operator<< (std::ostream& o, const MetavarMap<Data>& t);
}

# include <parser/metavar-map.hxx>

#endif // !PARSER_METAVAR_MAP_HH

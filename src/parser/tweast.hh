/**
 ** \file parser/tweast.hh
 ** \brief Declaration of parser::Tweast.
 */

#ifndef PARSER_TWEAST_HH
# define PARSER_TWEAST_HH

# include <iosfwd>
# include <sstream>

# include <libport/map.hh>
# include <libport/symbol.hh>
# include <libport/unique-pointer.hh>

# include "ast/fwd.hh"
# include "ast/symbols-type.hh"

# include "parser/metavar-map.hh"

namespace parser
{

  /// \brief TWEAST stands for ``Text With Embedded Abstract Syntax Trees''.
  ///
  /// Aggregate string to parse and tables of metavariables.
  class Tweast
    : public MetavarMap<ast::Call>,
      public MetavarMap<ast::Exp>,
      public MetavarMap<ast::exps_type>,
      public MetavarMap<ast::symbols_type>
  {
  public:
    Tweast ();
    Tweast (const std::string& str);
    virtual ~Tweast ();

    /// \brief Stream manipulator.
    ///
    /// Append Tiger expressions to the string to parse.
    ///
    /// \precondition: If \a t is an AST pointer type, it must not
    /// have been already registered.
    ///
    /// Registering the same address twice means that the
    /// same address will be used twice, and therefore deleted twice.
    ///
    /// Why don't we allow the use of a same node several times?
    /// Because most of the time it is wrong: it means you use a single
    /// ast several times, which means that something that was written
    /// once initially will be written twice in the result.  In that
    /// case you are likely to introduce several computations of the
    /// expressions (with possibly several times its side-effects)
    /// which is wrong (thing of Cpp macros).  You typically need to
    /// introduce a temporary in that case.
    ///
    /// But of course sometimes you really want to use that tree
    /// several times.  In which case explicitly clone it on the call
    /// side.
    template <typename T> Tweast& operator<< (const T& t);

    /// Metavariables manipulator.
    template <typename T> T* take (unsigned s) throw (std::range_error);

    /// Get the current input string.
    std::string input_get () const;

    /// Print the table
    std::ostream& dump (std::ostream& ostr) const;

  protected:
    /// Store some typed data.
    /// \{
    /// Define virtual std::string append_ (unsigned& key, ast::Call* data);
    using MetavarMap<ast::Call>::append_;
    using MetavarMap<ast::Exp>::append_;
    using MetavarMap<ast::exps_type>::append_;
    using MetavarMap<ast::symbols_type>::append_;
    template <typename T> T& append_ (unsigned&, T& data) const;
    /// \}

    /// Whether the pointer must be registered only once.
    /// \{
    using MetavarMap<ast::Call>::must_be_unique_;
    using MetavarMap<ast::Exp>::must_be_unique_;
    using MetavarMap<ast::exps_type>::must_be_unique_;
    using MetavarMap<ast::symbols_type>::must_be_unique_;
    template <typename T> bool must_be_unique_ (const T&) const;
    /// \}

  protected:
    /// The next identifier suffix to create.
    static unsigned count_;

    /// The string to parse.
    std::stringstream input_;
# ifndef NDEBUG
    /// The set of pointers that must be unique.
    libport::UniquePointer unique_;
# endif
  };

  /// Display the content of the tweast.
  std::ostream& operator<< (std::ostream& ostr, const Tweast& in);

}

# include "parser/tweast.hxx"

#endif // !PARSER_TWEAST_HH

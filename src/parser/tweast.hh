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

# include <ast/fwd.hh>
# include <ast/symbols-type.hh>

# include <parser/metavar-map.hh>

namespace parser
{

# define TWEAST_META_VARS                       \
  (ast::rCall)                                  \
  (ast::rExp)                                   \
  (ast::exps_type*)                             \
  (ast::symbols_type*)

  /// \brief TWEAST stands for ``Text With Embedded Abstract Syntax Trees''.
  ///
  /// Aggregate string to parse and tables of metavariables.
  class Tweast
    : public MetavarMap<ast::rCall>
    , public MetavarMap<ast::rExp>
    , public MetavarMap<ast::exps_type*>
    , public MetavarMap<ast::symbols_type*>
  {
  public:
    Tweast ();
    Tweast (const std::string& str);
    virtual ~Tweast ();

    /// \brief Stream manipulator.
    ///
    /// Append expressions to the string to parse.
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
    template <typename T> T take (unsigned s) throw (std::range_error);

    /// Get the current input string.
    std::string input_get () const;

    /// Print the table
    std::ostream& dump (std::ostream& ostr) const;

  protected:
    /// Store some typed data.
    template <typename T> T& append_ (unsigned&, T& data) const;

    /// Whether the pointer must be registered only once.
    template <typename T> bool must_be_unique_ (const T&) const;

# define TWEAST_USING_META_VARIABLE(R, Data, Elem)      \
    using MetavarMap<Elem>::append_;                    \
    using MetavarMap<Elem>::must_be_unique_;
    BOOST_PP_SEQ_FOR_EACH(TWEAST_USING_META_VARIABLE, ~, TWEAST_META_VARS);
# undef TWEAST_USING_META_VARIABLE

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

# include <parser/tweast.hxx>

#endif // !PARSER_TWEAST_HH

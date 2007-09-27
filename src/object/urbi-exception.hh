/**
 ** \file object/urbi-exception.hh
 ** \brief Definition of UrbiException
 */

#ifndef OBJECT_URBI_EXCEPTION_HH
# define OBJECT_URBI_EXCEPTION_HH
# include <exception>
# include <string>

# include "ast/loc.hh"
# include "object/fwd.hh"
# include "object/object.hh"

namespace object
{
  /// This class defines an exception used when an error
  /// occurs in an URBI primitive.
  class UrbiException : public std::exception
  {
  public:
    /// Destructor.
    virtual ~UrbiException () throw ();

    /// \brief Construct an UrbiException (used when lookup fails).
    /// \param slot Searched slot.
    static UrbiException lookupFailed (std::string slot);

    /// \brief Construct an UrbiException from a primitive and a message.
    /// \param primitive primitive which has thrown the error.
    /// \param msg error message which will be sent.
    static UrbiException primitiveError (std::string primitive,
					 std::string msg);

    /// Construct an UrbiException (used when types mismatch in a primitive).
    /// \param real Real type.
    /// \param expected Expected type.
    /// \param loc Error's location.
    static UrbiException wrongArgumentType (Object::kind_type real,
					    Object::kind_type expected);

    /// Construct an UrbiException (used when args count is wrong).
    /// \param argReal Number of arguments given.
    /// \param argExpected Number of arguments expected.
    static UrbiException wrongArgumentCount (unsigned argReal,
					     unsigned argExpected);

    /// Return the exception's error message.
    virtual const char* what () const throw ();

    /// Get location.
    const ast::loc& location_get () const;

    /// Set location.
    void location_set (const ast::loc&);

  protected:
    /// \name Constructor
    /// \brief Construct an exception which contains a raw message.
    /// \param msg raw error message
    explicit UrbiException (std::string msg);

    /// \name Constructor
    /// \brief Construct an exception which contains a raw message.
    /// \param msg raw Error message.
    /// \param loc Error's location.
    explicit UrbiException (std::string msg, const ast::loc& loc);

  private:
    /// Error message.
    std::string msg_;

    /// Location
    ast::loc loc_;

    /// Error messages.
    static const char lookupFailed_[];
    static const char primitiveError_[];
    static const char wrongArgumentType_[];
    static const char wrongArgumentCount_[];
  };

  /// Throw an exception if formal != effective.
  /// \note: self is not included in the count.
  void check_arg_count (unsigned formal, unsigned effective);

} // namespace object

# include "object/urbi-exception.hxx"
#endif //! OBJECT_URBI_EXCEPTION_HH

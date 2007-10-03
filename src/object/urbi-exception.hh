/**
 ** \file object/urbi-exception.hh
 ** \brief Definition of UrbiException
 */

#ifndef OBJECT_URBI_EXCEPTION_HH
# define OBJECT_URBI_EXCEPTION_HH

# include <exception>
# include <string>

# include "libport/symbol.hh"

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

    /// Return the exception's error message.
    virtual const char* what () const throw ();

    /// Get location.
    const ast::loc& location_get () const;

    /// Set location.
    void location_set (const ast::loc&);

  protected:
    /**
     * \brief Construct an exception which contains a raw message.
     * \param msg raw error message.  */
    explicit UrbiException (std::string msg);

    /**
     * \brief Construct an exception which contains a raw message.
     * \param msg raw Error message.
     * \param loc Error's location.  */
    UrbiException (std::string msg, const ast::loc& loc);

  private:
    /// Error message.
    std::string msg_;

    /// Location
    ast::loc loc_;
  };


  /** Exception for lookup failures.
   * \param slot Searched slot.  */
  struct LookupError: public UrbiException
  {
    explicit LookupError (libport::Symbol slot);
  };

  /// Explicit for slots redefined.
  struct RedefinitionError: public UrbiException
  {
    explicit RedefinitionError (libport::Symbol slot);
  };

  /** Exception for errors related to primitives usage.
   * \param primitive primitive which has thrown the error.
   * \param msg error message which will be sent.  */
  struct PrimitiveError: public UrbiException
  {
    PrimitiveError (std::string primitive,
                    std::string msg);
  };

  /** Exception for type mismatch in a primitive usage.
   * \param real Real type.
   * \param expected Expected type.
   * \param loc Error's location.  */
  struct WrongArgumentType: public UrbiException
  {
    WrongArgumentType (Object::kind_type real,
                       Object::kind_type expected);

  };

  /** Exception used for calls with wrong argument count.
   * \param argReal Number of arguments given.
   * \param argExpected Number of arguments expected.  */
  struct WrongArgumentCount: public UrbiException
  {
    WrongArgumentCount (unsigned argReal,
                        unsigned argExpected);
  };

  /// Throw an exception if formal != effective.
  /// \note: \c self is not included in the count.
  void check_arg_count (unsigned formal, unsigned effective);

} // namespace object

# include "object/urbi-exception.hxx"
#endif //! OBJECT_URBI_EXCEPTION_HH

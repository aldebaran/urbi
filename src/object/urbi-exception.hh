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

    /// Initialize message (add debug information if required).
    void initialize_msg () throw ();

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
    explicit UrbiException (const std::string& msg);

    /**
     * \brief Construct an exception which contains a raw message.
     * \param msg raw Error message.
     * \param loc Error's location.  */
    UrbiException (const std::string& msg, const ast::loc& loc);

    /**
     * \brief Construct an exception which contains a raw message.
     * \param msg raw Error message.
     * \param fun C++ function that raised.  */
    UrbiException (const std::string& msg,
		   const std::string& fun);

  private:
    /// Error message.
    std::string msg_;

    /// Location
    ast::loc loc_;

    /// The (C++) function that raised.
    std::string fun_;
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
   * \param primitive   primitive that has thrown the error.
   * \param msg         error message which will be sent.  */
  struct PrimitiveError: public UrbiException
  {
    PrimitiveError (const std::string& primitive,
		    const std::string& msg);
  };

  /** Exception for type mismatch in a primitive usage.
   * \param formal      Expected type.
   * \param effective   Real type.
   * \param fun         Primitive's name.  */
  struct WrongArgumentType: public UrbiException
  {
    WrongArgumentType (Object::kind_type formal,
		       Object::kind_type effective,
		       const std::string& fun);
    /// Invalid use of void.
    WrongArgumentType(const std::string& fun);

  };

  /** Exception used for calls with wrong argument count.
   * \param effective  Number of arguments given.
   * \param formal     Number of arguments expected.  */
  struct WrongArgumentCount: public UrbiException
  {
    WrongArgumentCount (unsigned formal, unsigned effective,
			const std::string& fun);
  };

  /// Throw an exception if formal != effective.
  /// \note: \c self is not included in the count.
  void check_arg_count (unsigned formal, unsigned effective,
		       const std::string& fun);

} // namespace object

# include "object/urbi-exception.hxx"
#endif //! OBJECT_URBI_EXCEPTION_HH

/**
 ** \file object/urbi-exception.hh
 ** \brief Definition of UrbiException
 */

#ifndef OBJECT_URBI_EXCEPTION_HH
# define OBJECT_URBI_EXCEPTION_HH

# include <string>

# include <libport/ufloat.hh>
# include <libport/symbol.hh>

# include <ast/fwd.hh>
# include <ast/call.hh>
# include <ast/loc.hh>

# include <kernel/exception.hh>

# include <object/fwd.hh>
# include <object/object-kind.hh>

namespace object
{
  /// This class defines an exception used when an error
  /// occurs in an URBI primitive.
  class UrbiException : public kernel::exception
  {
  public:
    /// Destructor.
    virtual ~UrbiException () throw ();

    /// Initialize message (add debug information if required).
    void initialize_msg () throw ();

    /// Return the exception's error message.
    virtual std::string what () const throw ();

    /// The call stack
    typedef std::vector<ast::rConstCall> call_stack_type;

    ADD_FIELD (ast::loc, location)
    ADD_FIELD (call_stack_type, backtrace)
    ADD_FIELD (std::string, msg)
    ADD_FIELD (std::string, function);

  protected:
    /**
     * \brief Construct an exception which contains a raw message.
     * \param msg raw error message.  */
    explicit UrbiException (const std::string& msg);

    /**
     * \brief Construct an exception which contains a raw message.
     * \param msg raw Error message.
     * \param loc Error's location.  */
    UrbiException (const std::string& msg, const ast::loc&);

    /**
     * \brief Construct an exception which contains a raw message.
     * \param msg raw Error message.
     * \param fun C++ function that raised.  */
    UrbiException (const std::string& msg,
		   const std::string& fun);

    COMPLETE_EXCEPTION (UrbiException)
  };


  /** Exception for lookup failures.
   * \param slot Searched slot.  */
  struct LookupError: public UrbiException
  {
    explicit LookupError (libport::Symbol slot);
    COMPLETE_EXCEPTION (LookupError)
  };

  /// Explicit for slots redefined.
  struct RedefinitionError: public UrbiException
  {
    explicit RedefinitionError (libport::Symbol slot);
    COMPLETE_EXCEPTION (RedefinitionError)
  };

  /// Exception raised when the stack space in a task is almost exhausted
  struct StackExhaustedError: public UrbiException
  {
    explicit StackExhaustedError (const std::string& msg);
    COMPLETE_EXCEPTION (StackExhaustedError)
  };

  /** Exception for errors related to primitives usage.
   * \param primitive   primitive that has thrown the error.
   * \param msg         error message which will be sent.  */
  struct PrimitiveError: public UrbiException
  {
    PrimitiveError (const std::string& primitive,
		    const std::string& msg);
    COMPLETE_EXCEPTION (PrimitiveError)
  };

  /** Exception for type mismatch in a primitive usage.
   * \param formal      Expected type.
   * \param effective   Real type.
   * \param fun         Primitive's name.  */
  struct WrongArgumentType: public UrbiException
  {
    WrongArgumentType (object_kind_type formal,
		       object_kind_type effective,
		       const std::string& fun);
    /// Invalid use of void.
    WrongArgumentType(const std::string& fun);

    COMPLETE_EXCEPTION (WrongArgumentType)
  };

  /** Exception used for calls with wrong argument count.
   * \param effective  Number of arguments given.
   * \param formal     Number of arguments expected.
   * A version of the constructor also exists for functions taking a
   * variable number of arguments, between \param minformal and
   * \param maxformal.
   */
  struct WrongArgumentCount: public UrbiException
  {
    WrongArgumentCount (unsigned formal, unsigned effective,
			const std::string& fun);
    WrongArgumentCount (unsigned minformal, unsigned maxformal,
			unsigned effective, const std::string& fun);
    COMPLETE_EXCEPTION (WrongArgumentCount)
  };

  /** Exception used when a non-integer is provided to a primitive expecting
   * an integer.
   * \param effective  Effective floating point value that failed conversion.
   */
  struct BadInteger: public UrbiException
  {
    BadInteger (libport::ufloat effective, const std::string& fun);
    COMPLETE_EXCEPTION (BadInteger)
  };

  /** Exception used when building an implicit tag name (k1 style).
   */
  struct ImplicitTagComponentError: public UrbiException
  {
    ImplicitTagComponentError (const ast::loc&);
    COMPLETE_EXCEPTION (ImplicitTagComponentError);
  };

  /** Exception used when a non-interruptible block of code tries to
   * get interrupted or blocked.
   */
  struct SchedulingError: public UrbiException
  {
    SchedulingError (const std::string& msg);
    COMPLETE_EXCEPTION (SchedulingError);
  };

  /** Exception used for a path which should be impossible to take,
   * for example when a template method doesn't make sense.
   */
  struct InternalError: public UrbiException
  {
    InternalError (const std::string& msg);
    COMPLETE_EXCEPTION (InternalError);
  };

  /// Throw an exception if formal != effective.
  /// \note: \c self is included in the count.
  void check_arg_count (unsigned formal, unsigned effective,
		       const std::string& fun);

  /// Same as above, with a minimum and maximum number of
  /// formal parameters.
  void check_arg_count (unsigned minformal, unsigned maxformal,
			unsigned effective, const std::string& fun);

} // namespace object

# include <object/urbi-exception.hxx>
#endif //! OBJECT_URBI_EXCEPTION_HH

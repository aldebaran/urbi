/**
 ** \file object/urbi-exception.hh
 ** \brief Definition of Exception
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
# include <object/object.hh>

namespace object
{
  /// The call stack
  typedef std::pair<libport::Symbol,
                    boost::optional<ast::loc> > call_type;
  typedef std::vector<call_type> call_stack_type;

  /// Urbi-visible exceptions
  class UrbiException: public kernel::exception
  {
  public:
    UrbiException(rObject value, const call_stack_type& bt);
    rObject value_get();
    const call_stack_type& backtrace_get();

  private:
    rObject value_;
    call_stack_type bt_;
    COMPLETE_EXCEPTION(UrbiException);
  };

  /// This class defines an exception used when an error
  /// occurs in an URBI primitive.
  class Exception : public kernel::exception
  {
  public:
    /// Destructor.
    virtual ~Exception() throw ();

    /// Initialize message (add debug information if required).
    void initialize_msg() throw ();

    /// Return the exception's error message.
    virtual std::string what() const throw ();

    /// Returns true if the exception was allready displayed.
    bool was_displayed() const;

    /// Mark the exception as allready displayed.
    void set_displayed();

    ADD_FIELD(ast::loc, location)
    ADD_FIELD(call_stack_type, backtrace)
    ADD_FIELD(std::string, msg)
    ADD_FIELD(libport::Symbol, function);

  protected:
    /**
     * \brief Construct an exception which contains a raw message.
     * \param msg raw error message.  */
    explicit Exception(const std::string& msg);

    /**
     * \brief Construct an exception which contains a raw message.
     * \param msg raw Error message.
     * \param loc Error's location.  */
    Exception(const std::string& msg, const ast::loc&);

    /**
     * \brief Construct an exception which contains a raw message.
     * \param msg raw Error message.
     * \param fun C++ function that raised.  */
    Exception(const std::string& msg, const libport::Symbol fun);

    private:
    /// Was the exception displayed
    bool displayed_;
    COMPLETE_EXCEPTION(Exception)
  };

} // namespace object

# include <object/urbi-exception.hxx>
#endif //! OBJECT_URBI_EXCEPTION_HH

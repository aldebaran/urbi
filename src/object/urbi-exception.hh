/**
 ** \file object/urbi-exception.hh
 ** \brief Definition of UrbiException
 */

#ifndef OBJECT_URBI_EXCEPTION_HH
# define OBJECT_URBI_EXCEPTION_HH

# include <exception>
# include <string>

namespace object
{
  /// This class defines an exception used when an error
  /// occurs in an URBI primitive.
  class UrbiException : public std::exception
  {
    /// \name Ctor & dtor.
    /// \{
  public:
    /// \brief Construct an UrbiException from a primitive and a message.
    /// \param primitive primitive which has thrown the error.
    /// \param msg error message which will be sent.
    explicit UrbiException (std::string primitive, std::string msg);

    /// \brief Contrusct an exception which contains a raw message.
    /// \param msg raw error message
    explicit UrbiException (std::string msg);

    virtual ~UrbiException () throw ();

    /// \}

    /// Return the exception's error message.
    virtual const char* what () const throw ();

  private:
    /// Error message.
    std::string msg_;
  };
}; // namespace object


#endif //! OBJECT_URBI_EXCEPTION_HH

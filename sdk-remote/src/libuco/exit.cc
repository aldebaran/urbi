#include <urbi/exit.hh>

namespace urbi
{
  Exit::Exit(int error, const std::string& message)
      : error_(error)
      , msg_(message)
  {}

  Exit::~Exit() throw ()
  {}

  const char* Exit::what() const throw ()
  {
    return msg_.c_str();
  }

  int Exit::error_get() const
  {
    return error_;
  }
}


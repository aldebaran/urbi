#include <sstream>

#include <parser/parser-utils.hh>

namespace parser
{

  std::string
  message_format(const yy::parser::location_type& l,
		 const std::string& msg)
  {
    std::ostringstream o;
    o << "!!! " << l << ": " << msg;
    return o.str();
  }

} // namespace parser

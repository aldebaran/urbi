#ifndef PARSER_TRANSFORM_HH
# define PARSER_TRANSFORM_HH

# include <ast/fwd.hh>
# include <urbi/export.hh>

namespace parser
{
  template <typename T>
  URBI_SDK_API libport::intrusive_ptr<T> transform(libport::intrusive_ptr<T> ast);
} // namespace parser

#endif // PARSER_TRANSFORM_HH

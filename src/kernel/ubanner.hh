#ifndef KERNEL_UBANNER_HH
# define KERNEL_UBANNER_HH

# include <libport/fwd.hh>
# include <urbi/export.hh>

namespace kernel
{
  URBI_SDK_API std::ostream&
  userver_package_info_dump(std::ostream& o);
}
#endif // !KERNEL_UBANNER_HH

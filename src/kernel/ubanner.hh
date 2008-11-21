#ifndef KERNEL_UBANNER_HH
# define KERNEL_UBANNER_HH

# include <libport/fwd.hh>
# include <urbi/export.hh>
/// To display at the start of a session.
extern URBI_SDK_API const char* HEADER_BEFORE_CUSTOM[];

/// To display at the end of a session.
extern URBI_SDK_API const char* HEADER_AFTER_CUSTOM[];

/// To display in the middle of the uconsole version info.
extern URBI_SDK_API const char* uconsole_banner[];

URBI_SDK_API std::ostream&
userver_package_info_dump (std::ostream& o);
#endif // !KERNEL_UBANNER_HH

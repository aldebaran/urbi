#ifndef UBANNER_HH
# define UBANNER_HH

# include <libport/fwd.hh>

/// To display at the start of a session.
extern const char* HEADER_BEFORE_CUSTOM[];

/// To display at the end of a session.
extern const char* HEADER_AFTER_CUSTOM[];

/// To display in the middle of the uconsole version info.
extern const char* uconsole_banner[];

std::ostream& userver_package_info_dump (std::ostream& o);
#endif //!UBANNER_HH

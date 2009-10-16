#ifndef URBI_ROOT_HH
# define URBI_ROOT_HH

# include <string>

extern const char* lib_rel_path;
extern const char* lib_ext;
extern const char* LD_LIBRARY_PATH_NAME;

std::string get_urbi_root(const char* arg0);

#endif

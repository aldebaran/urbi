/// \file urbi/umain.hh

#ifndef URBI_UMAIN_HH
# define URBI_UMAIN_HH

# include <urbi/uobject.hh>

#  define UMAIN()				\
  int						\
  main(int argc, const char *argv[])		\
  {						\
    urbi::main(argc, argv, true);		\
  }

#endif /* !URBI_UMAIN_HH */

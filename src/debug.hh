#ifndef DEBUG_HH
# define DEBUG_HH

#  define DEBUG(Flag, Msg)					\
  do                                                            \
  {								\
    std::cerr << "\x1b\x5b\x33\x33\x6d"                         \
              << Flag << ": "                                   \
              << "\x1b\x5b\x6d"                                 \
              << libport::indent << Msg << std::endl;           \
    SLEEP(1);							\
  } while (0)

#  define DEBUGN(Flag, Msg)					\
  do                                                            \
  {								\
    std::cerr << Msg;                                           \
    SLEEP(1);							\
  } while (0)

#endif

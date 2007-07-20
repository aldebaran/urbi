/**
 ** \file object/object-test.hh
 ** \brief Definition for testing object::Object.
 */

#ifndef OBJECT_TEST_HH_
# define OBJECT_TEST_HH_

# include <string>

# include "object/object.hh"
# include "object/atom.hh"

# define ECHO(This) std::cerr << This << std::endl
# define ECHO2(Msg, This)			\
  do						\
  {						\
    if (std::string (Msg) != std::string (""))	\
      std::cerr << Msg << " : ";		\
    std::cerr << This << std::endl;		\
  } while (0)
# define ECHOVAL(Obj, Val)			\
do						\
{						\
  rObject Obj ## _ ## Val = (*Obj).lookup (#Val);	\
  ECHO2(#Obj "_" #Val, *Obj ## _ ## Val);		\
} while (0)

# define NEWLINE()  std::cerr << std::endl

// We don't want to link against libkernel.la, but we use some of its
// primitives.
struct UServer
{
  void shutdown () { std::exit(0); };
  void reboot ()   { std::exit(1); };
};

UServer urbiserver;

#endif // !OBJECT_TEST_HH_

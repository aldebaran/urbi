// Microsoft compiler does not allow main to be in a library.
// So we define one here.

#include <cerrno>
#include <iostream>
#include <vector>

#include <config.h>

#include <libport/cstdlib>

#include <libport/compiler.hh>
#include <libport/containers.hh>
#include <libport/detect-win32.h>
#include <libport/foreach.hh>
#include <libport/path.hh>
#include <libport/program-name.hh>
#include <libport/sysexits.hh>
#include <libport/unistd.h>

ATTRIBUTE_NORETURN
void
exec(std::vector<const char*> args)
{
  args.push_back(0);
  char* const* argv = const_cast<char* const*>(&args[0]);
  execv(argv[0], argv);

  std::cerr << libport::program_name()
            << ": failed to invoke " << args[0]
            << ": " << strerror(errno)
            << std::endl
            << libport::exit(EX_OSFILE);
}

int
main(int argc, char* argv[])
{
  libport::program_initialize(argc, argv);
  // Installation prefix.
  std::string urbi_root = libport::xgetenv("URBI_ROOT", URBI_ROOT);

  const char* URBI_LAUNCH = getenv("URBI_LAUNCH");
  std::string urbi_launch = (URBI_LAUNCH
                             ? URBI_LAUNCH
                             : urbi_root + "/bin/urbi-launch" EXEEXT);

  std::vector<const char*> args;
  args.reserve(3 + argc + 1);
  args << urbi_launch.c_str() << "--start" << "--";
  args.insert(args.end(), argv + 1, argv + argc);
  exec(args);
}

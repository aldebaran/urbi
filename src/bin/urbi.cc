#include <config.h>

#include <vector>

#include <libport/cstdlib>

#include <libport/containers.hh>
#include <libport/debug.hh>
#include <libport/detect-win32.h>
#include <libport/program-name.hh>
#include <libport/separate.hh>
#include <libport/unistd.h>

GD_INIT();
GD_ADD_CATEGORY(urbi);

int
main(int argc, char* argv[])
{
  libport::program_initialize(argc, argv);
  // Installation prefix.
  std::string urbi_root = libport::xgetenv("URBI_ROOT", URBI_ROOT);
  std::string urbi_launch =
    libport::xgetenv("URBI_LAUNCH", urbi_root + "/bin/urbi-launch" EXEEXT);
  std::vector<const char*> args;
  args.reserve(3 + argc + 1);
  libport::OptionParser opt_parser;
  opt_parser << libport::opts::debug;
  opt_parser(libport::program_arguments());
  args << urbi_launch.c_str()
       << "--start";
  if (libport::opts::debug.filled())
    args << "--debug" << libport::opts::debug.value().c_str();
  args << "--";
  args.insert(args.end(), argv + 1, argv + argc);
  GD_CATEGORY(urbi);
  GD_FINFO_DEBUG("exec: %s", (libport::separate(args, " ")));
  libport::exec(args);
}

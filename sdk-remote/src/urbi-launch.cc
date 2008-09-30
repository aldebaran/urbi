#include <ltdl.h>
#include <string>
#include <cassert>
#include <iostream>
#include <boost/filesystem.hpp>

#include <sdk/config.h>

#include <libport/foreach.hh>
#include <libport/unistd.h>

#include <urbi/uclient.hh>

using namespace urbi;
using namespace boost::filesystem;

std::string host = URBI_HOST;
std::string dynld = URBI_SHREXT;
const char* umain_sym = "urbi_main";
enum ConnectMode {
  /// Start a new engine and plug the module
  MODE_PLUGIN_START,
  /// Load the module in a running engine as a plugin
  MODE_PLUGIN_LOAD,
  /// Connect the module to a running engine (remote uobject)
  MODE_REMOTE
};
UCallbackAction
onError(const UMessage& msg)
{
  std::cerr <<"load module error: " << msg.message << std::endl;
  return URBI_CONTINUE;
}

UCallbackAction
onDone(const UMessage&)
{
  ::exit(0);
}

int connect_plugin(int argc, const char* argv[],
                   std::list<std::string>& modules)
{
  std::string host = "localhost";
  int port = 54000;
  for (int i=1; i< argc-1; i++)
  {
    std::string arg = argv[i];
    if (arg == "-H" || arg == "--host")
      host = argv[++i];
    else if (arg == "-P" || arg == "--port")
      port = strtol(argv[++i], 0, 0);
  }
  UClient cl(host.c_str(), port);
  if (cl.error())
    ::exit(1); // UClient allready displayde an error message
  cl.setErrorCallback(callback(&onError));
  cl.setCallback(callback(&onDone), "output");
  foreach(const std::string& m, modules)
    cl << "loadModule(\"" << m << "\");";
  cl << "output << 1;";
  while(true)
    sleep(1);
  return 0;
}

void usage(const char* name)
{
  std::cerr
  << "usage: " << name << " MODE MODULENAMES ... [OPTIONS]\n"
  << "    Start an UObject in either remote or plugin mode\n"
  << "\n"
  << "Possible values for MODE:\n"
  << "  -r, --remote      Start as a remote uobject\n"
  << "  -p, --plugin      Start as a plugin uobject on a running server\n"
  << "  -s, --start       Start an urbi server and connect as plugin\n"
  << "\n"
  << "Options for plugin mode:\n"
  << "  -H, --host             Server host name\n"
  << "  -p, --port             Server port\n"
  << "Options for remote and start mode are passed to urbi::main\n"
  << "\n"
  << "MODULENAMES is a list of modules and directory which will be searched\n"
  << "  for modules.\n"
  << std::endl;
  ::exit(1);
}

static void list_modules(path p, int recurse, std::list<std::string> &res)
{
  if (is_regular(p))
  {
    if (extension(p) != dynld)
      throw std::runtime_error(
         "File " + p.string() + " does not look like a shared library");
    if (!p.is_complete())
      p = current_path() / p;
    res.push_back(p.string());
  }
  else if (is_directory(p))
  {
    if (recurse)
    {
      directory_iterator end_itr; // default construction yields past-the-end
      for ( directory_iterator itr( p );
           itr != end_itr;
           ++itr )
      {
        list_modules(*itr, recurse-1, res);
      }
    }
  }
  else
    throw std::runtime_error("File not found: " + p.string());
}

typedef int (*umain_type)(int argc, const char* argv[], int block);
int main(int argc, const char* argv[])
{
  lt_dlinit();
  std::string prefix=URBI_PREFIX;

  int argp = 1;


  if (argc < 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
    usage(argv[0]);

  /// Parse mode
  ConnectMode connectMode = MODE_REMOTE;
  std::string mode = argv[argp++];
  /// Be leniant, remove all '-'.
  while (mode.length() && mode[0] == '-')
    mode = mode.substr(1, mode.npos);
  /// Be extra-leniant, only check first letter.
  switch(mode[0])
  {
  case 'r':
    connectMode = MODE_REMOTE;
    break;
  case 's':
    connectMode = MODE_PLUGIN_START;
    break;
  case 'p':
    connectMode = MODE_PLUGIN_LOAD;
    break;
  default:
    std::cerr << "Invalid mode '" << argv[1] << "'" << std::endl;
    usage(argv[0]);
    break;
  }

   /// Get the list of modules.
  std::list<std::string> modules;
  for (; argp < argc && argv[argp][0] != '-'; ++argp)
    list_modules(argv[argp], 1, modules);

  /// Store args, intercepting host and port just in case
  const char** nargv = new const char*[argc];
  int nargp = 1;
  nargv[0] = argv[0];
  while (argp < argc)
    nargv[nargp++] = argv[argp++];

  if (connectMode == MODE_PLUGIN_LOAD)
    return connect_plugin(nargp, nargv, modules);

  /* The two other modes are handled the same way:
   * -Dlopen the correct libuobject.
   * -Dlopen the uobjects to load.
   * -Call urbi::main found by dlsym() in libuobject.
   */
  std::string dll = prefix + "/gostai/core/" + host + '/'
    + (connectMode == MODE_REMOTE?"remote":"engine") + "/libuobject.so";
  std::cerr << "loading " << dll << std::endl;
  lt_dlhandle core = lt_dlopen(dll.c_str());
  if (!core)
  {
    std::cerr << "Failed to load core: " << lt_dlerror() << std::endl;
    ::exit(1);
  }

  foreach(const std::string& s, modules)
  {
    lt_dlhandle uobject = lt_dlopen(argv[2]);
    if (!uobject)
    {
      std::cerr << "Failed to load " << s << ": " << lt_dlerror() << std::endl;
      ::exit(1);
    }
  }
  umain_type umain = (umain_type) lt_dlsym(core, umain_sym);
  if (!umain)
  {
    std::cerr << "Failed to dlsym urbi::main: " << lt_dlerror() << std::endl;
    ::exit(1);
  }
  umain(nargp, nargv, true);
}

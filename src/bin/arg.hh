#ifndef ARG_HH
# define ARG_HH

# include <fstream>

static void fatal(const std::string& msg)
{
  std::cerr << msg << std::endl;
  exit(1);
}

static std::istream* i_delete = 0;
static std::ostream* o_delete = 0;

static void cleanup()
{
  delete i_delete;
  delete o_delete;
}

static bool
get_arg(char s, const std::string& l, std::string& res, int& argc, char** argv)
{
  for (int i = 0; i < argc; ++i)
    if (argv[i] == std::string("-") + s || argv[i] == "--" + l)
    {
      if (i + 1 == argc)
        fatal("Option requires an argument: " + l);
      res = argv[i + 1];
      argc -= 2;
      for (int j = i; j < argc; ++j)
        argv[j] = argv[j + 2];
      i--;
      return true;
    }
  return false;
}

static void
get_io(std::istream*& input, std::ostream*& output,
       int& argc, char** argv)
{
  std::string res;
  if (get_arg('i', "input", res, argc, argv))
  {
    input = i_delete = new std::ifstream(res.c_str());
    if (!input->good())
      fatal("Unable to read input file: " + res);
  }
  else
    input = &std::cin;
  if (get_arg('o', "output", res, argc, argv))
  {
    output = o_delete = new std::ofstream(res.c_str());
    if (!output->good())
      fatal("Unable to read output file: " + res);
  }
  else
    output = &std::cout;
  atexit(cleanup);
}

#endif

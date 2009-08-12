#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <libport/unistd.h>

#include <object/input-stream.hh>
#include <object/output-stream.hh>
#include <object/process.hh>
#include <object/symbols.hh>

namespace object
{
  Process::Process(const std::string& binary,
                   const arguments_type& argv)
    : pid_(0)
    , joined_(false)
    , binary_(binary)
    , argv_(argv)
    , stream_(0)
  {
    proto_add(proto ? proto : Object::proto);
  }

  Process::Process(rProcess model)
    : pid_(0)
    , joined_(false)
    , binary_(model->binary_)
    , argv_(model->argv_)
    , stream_(0)
  {
    proto_add(model);
  }

  void
  Process::init(const std::string& binary,
                const arguments_type& argv)
  {
    binary_ = binary;
    argv_ = argv;
  }

  Process::~Process()
  {
    if (stream_)
      delete stream_;
    if (pid_)
    {
      close(input_fd_[0]);
      close(input_fd_[1]);
      close(output_fd_[0]);
      close(output_fd_[1]);
      // FIXME: collect the zombie!
      // if (!joined_)
    }
  }

  void Process::run()
  {
    if (pid_)
      RAISE("Process was already run");

    if (pipe(output_fd_))
      RAISE(libport::strerror(errno));
    if (pipe(input_fd_))
    {
      close(output_fd_[0]);
      close(output_fd_[1]);
      RAISE(libport::strerror(errno));
    }

    pid_ = fork();

    if (pid_ < 0)
    {
      close(input_fd_[0]);
      close(input_fd_[1]);
      close(output_fd_[0]);
      close(output_fd_[1]);
      pid_ = 0;
      RAISE(libport::strerror(errno));
    }

    if (pid_)
    {
      // Parent
      close(input_fd_[0]);
      close(output_fd_[1]);
      slot_set(SYMBOL(input), new OutputStream(input_fd_[1], false));
      slot_set(SYMBOL(output), new InputStream(output_fd_[0], false));
    }
    else
    {
      // Child
      close(input_fd_[1]);
      close(output_fd_[0]);
      if (dup2(output_fd_[1], STDOUT_FILENO) == -1)
      {
        libport::perror("dup2");
        std::abort();
      }
      if (dup2(input_fd_[0], STDIN_FILENO) == -1)
      {
        libport::perror("dup2");
        std::abort();
      }
      try
      {
        arguments_type argv;
        argv << binary_ << argv_;
        libport::exec(argv, true);
      }
      catch (...)
      { /* nothing */ }
      libport::perror("exec");
      std::abort();
    }
  }

  void
  Process::join()
  {
    if (joined_)
      // FIXME: This should definitely be autorized
      RAISE("Process has already been joined");
    joined_ = true;
    int status;
    waitpid(pid_, &status, 0);
  }

  rObject
  Process::proto_make()
  {
    static const std::string bin = "/bin/true";
    arguments_type argv;
    argv << bin;
    return new Process(bin, argv);
  }

  void
  Process::initialize(CxxObject::Binder<Process>& bind)
  {
    bind(SYMBOL(init), &Process::init);
    bind(SYMBOL(join), &Process::join);
    bind(SYMBOL(run),  &Process::run );
  }

  URBI_CXX_OBJECT_REGISTER(Process);
}


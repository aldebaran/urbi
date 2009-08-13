#include <errno.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

#include <libport/lockable.hh>
#include <libport/thread.hh>
#include <libport/unistd.h>

#include <object/input-stream.hh>
#include <object/output-stream.hh>
#include <object/process.hh>
#include <object/symbols.hh>
#include <urbi/sdk.hh>

namespace object
{
  typedef std::vector<rProcess> processes_type;
  static processes_type processes;
  static libport::Lockable mutex;

  void
  Process::monitor_children()
  {
    // FIXME: Go in sleep mode when there are no processes (with a condition)
    while (true)
    {
      {
        libport::BlockLock lock(mutex);
        processes_type::iterator it = processes.begin();
        while (it != processes.end())
        {
          if (waitpid((*it)->pid_, &(*it)->status_, WNOHANG))
            it = processes.erase(it);
          else
            ++it;
        }
      }
      sleep(1);
    }
  }

  Process::Process(const std::string& binary,
                   const arguments_type& argv)
    : pid_(0)
    , binary_(binary)
    , argv_(argv)
    , status_(-1)
  {
    proto_add(proto ? proto : Object::proto);
  }

  Process::Process(rProcess model)
    : pid_(0)
    , binary_(model->binary_)
    , argv_(model->argv_)
    , status_(-1)
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
    if (pid_)
    {
      close(stdin_fd_[1]);
      close(stdout_fd_[0]);
      close(stderr_fd_[0]);
    }
  }

  void Process::run()
  {
    if (pid_)
      RAISE("Process was already run");

    if (pipe(stdout_fd_))
      RAISE(libport::strerror(errno));
    if (pipe(stdin_fd_))
    {
      close(stdout_fd_[0]);
      close(stdout_fd_[1]);
      RAISE(libport::strerror(errno));
    }
    if (pipe(stderr_fd_))
    {
      close(stdout_fd_[0]);
      close(stdout_fd_[1]);
      close(stdin_fd_[0]);
      close(stdin_fd_[1]);
      RAISE(libport::strerror(errno));
    }

    pid_ = fork();

    if (pid_ < 0)
    {
      close(stdin_fd_[0]);
      close(stdin_fd_[1]);
      close(stdout_fd_[0]);
      close(stdout_fd_[1]);
      close(stderr_fd_[0]);
      close(stderr_fd_[1]);
      pid_ = 0;
      RAISE(libport::strerror(errno));
    }

    if (pid_)
    {
      // Parent
      close(stdin_fd_[0]);
      close(stdout_fd_[1]);
      close(stderr_fd_[1]);
      slot_set(SYMBOL(stdin), new OutputStream(stdin_fd_[1], false));
      slot_set(SYMBOL(stdout), new InputStream(stdout_fd_[0], false));
      slot_set(SYMBOL(stderr), new InputStream(stderr_fd_[0], false));
      {
        libport::BlockLock lock(mutex);
        processes.push_back(this);
      }
    }
    else
    {
      // Child
      // Ask to be killed when the parent dies
      prctl(PR_SET_PDEATHSIG, SIGKILL);
      close(stdin_fd_[1]);
      close(stdout_fd_[0]);
      close(stderr_fd_[0]);
      if (dup2(stdout_fd_[1], STDOUT_FILENO) == -1)
      {
        libport::perror("dup2");
        std::abort();
      }
      if (dup2(stderr_fd_[1], STDERR_FILENO) == -1)
      {
        libport::perror("dup2");
        std::abort();
      }
      if (dup2(stdin_fd_[0], STDIN_FILENO) == -1)
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
  Process::join() const
  {
    while (status_ == -1)
      urbi::yield_for(500000);
  }

  bool
  Process::done() const
  {
    return status_ != -1;
  }

  rObject
  Process::status() const
  {
    rObject res = new Object;
    res->proto_add(Object::proto);
    bool exited = false;
    bool signaled = false;
    bool done = false;
    if (status_ == -1)
    {
      res->slot_set(SYMBOL(asString), to_urbi(std::string("running")));
    }
    else
    {
      done = true;
      if (WIFEXITED(status_))
      {
        exited = true;
        int rv = WEXITSTATUS(status_);
        res->slot_set(SYMBOL(exitStatus), to_urbi(rv));
        res->slot_set(SYMBOL(asString),
                      to_urbi(libport::format("exited with status %s", rv)));
      }
      else
      {
        assert(WIFSIGNALED(status_));
        signaled = true;
        int sig = WTERMSIG(status_);
        res->slot_set(SYMBOL(exitSignal), to_urbi(sig));
        res->slot_set(SYMBOL(asString),
                      to_urbi(libport::format("killed by signal %s", sig)));
      }
    }


    res->slot_set(SYMBOL(done), to_urbi(done));
    res->slot_set(SYMBOL(exited), to_urbi(exited));
    res->slot_set(SYMBOL(signaled), to_urbi(signaled));

    return res;
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
    bind(SYMBOL(done), &Process::done);
    bind(SYMBOL(init), &Process::init);
    bind(SYMBOL(join), &Process::join);
    bind(SYMBOL(run),  &Process::run );
    bind(SYMBOL(status),  &Process::status );

    libport::startThread(boost::function0<void>(&Process::monitor_children));
  }

  URBI_CXX_OBJECT_REGISTER(Process);
}


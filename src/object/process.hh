#ifndef OBJECT_PROCESS_HH
# define OBJECT_PROCESS_HH

# include <libport/fd-stream.hh>

# include <object/cxx-object.hh>

namespace object
{
  class Process: public CxxObject
  {

  /*---------------------------.
  | Construction / Destruction |
  `---------------------------*/

  public:
    typedef std::vector<std::string> arguments_type;
    Process(const std::string& binary,
            const arguments_type& argv);
    Process(rProcess model);
    virtual ~Process();

  /*-------------.
  | Urbi methods |
  `-------------*/

  public:
    void init(const std::string& binary,
              const arguments_type& argv);
    void run();
    /// Run, writting stderr and stdout to filename
    void runTo(const std::string& filename);
    void join() const;
    void kill();
    bool done() const;
    rObject status() const;
    std::string name() const;
    std::string as_string() const;

  /*--------.
  | Details |
  `--------*/

  private:
    static void monitor_children();
    void run_(boost::optional<std::string> outFile
               = boost::optional<std::string>());
    std::string name_;
    pid_t pid_;
    std::string binary_;
    arguments_type argv_;
    int status_;
    int stdout_fd_[2];
    int stderr_fd_[2];
    int stdin_fd_[2];
    URBI_CXX_OBJECT(Process);
  };
}

#endif

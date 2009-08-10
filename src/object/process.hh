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
    void join();

  /*--------.
  | Details |
  `--------*/

  private:
    pid_t pid_;
    bool joined_;
    std::string binary_;
    arguments_type argv_;
    libport::FdStream* stream_;
    int output_fd_[2];
    int input_fd_[2];
    URBI_CXX_OBJECT(Process);
  };
}

#endif

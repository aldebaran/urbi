#ifndef OBJECT_INPUT_STREAM_HH
# define OBJECT_INPUT_STREAM_HH

# include <libport/cstdio>

# include <object/cxx-object.hh>

namespace object
{
  class InputStream: public CxxObject
  {

  /*---------------------------.
  | Construction / Destruction |
  `---------------------------*/

  public:
    InputStream(int fd, bool own);
    InputStream(rInputStream stream);
    ~InputStream();

  /*------------.
  | Data access |
  `------------*/

  private:
    int _get();
    bool _getBuffer();
    std::string _getSeparator(char sep, bool incl, bool& ok);
    int fd_;
    bool own_;
    unsigned char buffer_[BUFSIZ];
    unsigned pos_;
    size_t size_;

  /*-------------.
  | Urbi methods |
  `-------------*/

  public:
    void init(rFile f);
    rObject get();
    rObject getChar();
    rObject getLine();

  /*--------.
  | Details |
  `--------*/

  private:
    URBI_CXX_OBJECT(InputStream);
  };
}

#endif

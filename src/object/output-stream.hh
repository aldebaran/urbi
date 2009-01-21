#ifndef OBJECT_OUTPUT_STREAM_HH
# define OBJECT_OUTPUT_STREAM_HH

# include <object/cxx-object.hh>

namespace object
{
  class OutputStream: public CxxObject
  {

  /*---------------------------.
  | Construction / Destruction |
  `---------------------------*/

  public:
    OutputStream(std::ostream& stream, bool own = false);
    OutputStream(rOutputStream stream);
    ~OutputStream();

  /*-------------.
  | Urbi methods |
  `-------------*/

  public:
    void init(rFile f);
    rOutputStream put(unsigned char);
    void flush();

  /*--------.
  | Details |
  `--------*/

  protected:
    std::ostream* stream_;

  private:
    bool own_;
    URBI_CXX_OBJECT(OutputStream);
  };
}

#endif

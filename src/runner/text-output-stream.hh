#ifndef OBJECT_TEXT_OUTPUT_STREAM_HH
# define OBJECT_TEXT_OUTPUT_STREAM_HH

# include <object/output-stream.hh>

namespace object
{
  class TextOutputStream: public OutputStream
  {

  /*---------------------------.
  | Construction / Destruction |
  `---------------------------*/

  public:
    TextOutputStream(std::ostream& stream, bool own = false);
    TextOutputStream(rTextOutputStream stream);
    ~TextOutputStream();

  /*-------------.
  | Urbi methods |
  `-------------*/

  public:
    rTextOutputStream put(rObject o);

  /*--------.
  | Details |
  `--------*/

  private:
    URBI_CXX_OBJECT(TextOutputStream);
  };
}

#endif

#include <object/text-output-stream.hh>
#include <runner/raise.hh>

namespace object
{
  TextOutputStream::TextOutputStream(std::ostream& stream, bool own)
    : OutputStream(stream, own)
  {
    protos_set(new List);
    proto_add(proto ? proto : OutputStream::proto);
  }

  TextOutputStream::TextOutputStream(rTextOutputStream)
    : OutputStream(*reinterpret_cast<std::ostream*>(0), false)
  {
    protos_set(new List);
    proto_add(proto);
  }

  /*-------------.
  | Urbi methods |
  `-------------*/

  rTextOutputStream TextOutputStream::put(rObject o)
  {
    rString str = o->call(SYMBOL(asString))->as<String>();
    // FIXME: encoding
    *stream_ << str->value_get();
    return this;
  }

  /*--------------.
  | Urbi bindings |
  `--------------*/

  rObject TextOutputStream::proto_make()
  {
    // FIXME
    return new TextOutputStream(std::cout);
  }

  void TextOutputStream::initialize(object::CxxObject::Binder<object::TextOutputStream>& bind)
  {
    bind(SYMBOL(LT_LT), &TextOutputStream::put);
  }

  URBI_CXX_OBJECT_REGISTER(TextOutputStream);
}

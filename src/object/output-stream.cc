#include <fstream>

#include <boost/format.hpp>

#include <object/file.hh>
#include <object/output-stream.hh>
#include <object/symbols.hh>

#include <runner/raise.hh>

namespace object
{
  OutputStream::OutputStream(std::ostream& stream, bool own)
    : stream_(&stream)
    , own_(own)
  {
    proto_add(proto ? proto : Object::proto);
  }

  OutputStream::OutputStream(rOutputStream)
    : stream_(0)
    , own_(false)
  {
    proto_add(proto);
  }

  OutputStream::~OutputStream()
  {
    if (own_)
      delete stream_;
  }

  /*-------------.
  | Urbi methods |
  `-------------*/

  void OutputStream::init(rFile f)
  {
    libport::path path = f->value_get()->value_get();
    stream_ =
      new std::ofstream(path.to_string().c_str());
    if (!stream_->good())
    {
      boost::format fmt("Unable to open file for writing: %s");
      delete stream_;
      stream_ = 0;
      RAISE(str(fmt % path));
    }
    own_ = true;
  }

  rOutputStream OutputStream::putByte(unsigned char c)
  {
    stream_->put(c);
    return this;
  }

  void OutputStream::flush()
  {
    stream_->flush();
  }

  rOutputStream
  OutputStream::put(rObject o)
  {
    checkFD_();
    std::string str = o->call(SYMBOL(asString))->as<String>()->value_get();
    size_t str_size = str.size();
    size_t size = write(fd_, str.c_str(), str_size);
    assert_eq(size, str_size);
    (void)size;
    return this;
  }

  /*--------------.
  | Urbi bindings |
  `--------------*/

  rObject OutputStream::proto_make()
  {
    // FIXME
    return new OutputStream(std::cout);
  }

  void
  OutputStream::initialize(object::CxxObject::Binder<object::OutputStream>& bind)
  {
    bind(SYMBOL(LT_LT), &OutputStream::put    );
    bind(SYMBOL(flush), &OutputStream::flush  );
    bind(SYMBOL(init),  &OutputStream::init   );
    bind(SYMBOL(put),   &OutputStream::putByte);
  }

  URBI_CXX_OBJECT_REGISTER(OutputStream);
}

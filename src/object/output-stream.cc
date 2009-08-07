#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <fstream>

#include <boost/format.hpp>

#include <object/file.hh>
#include <object/output-stream.hh>
#include <object/symbols.hh>

#include <runner/raise.hh>

namespace object
{
  OutputStream::OutputStream(int fd, bool own)
    : fd_(fd)
    , own_(own)
  {
    proto_add(proto ? proto : Object::proto);
  }

  OutputStream::OutputStream(rOutputStream model)
    : fd_(model->fd_)
    , own_(false)
  {
    proto_add(proto);
  }

  OutputStream::~OutputStream()
  {
    if (own_ && fd_ != -1)
      if (::close(fd_))
        RAISE(libport::strerror(errno));
  }

  void
  OutputStream::checkFD_() const
  {
    if (fd_ == -1)
      RAISE("Stream is closed");
  }

  /*-------------.
  | Urbi methods |
  `-------------*/

  void OutputStream::init(rFile f)
  {
    libport::path path = f->value_get()->value_get();
    fd_ = open(path.to_string().c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRWXU);

    if (fd_ < 0)
    {
      boost::format fmt("Unable to open file for writing: %s");
      fd_ = 0;
      RAISE(str(fmt % path));
    }
    own_ = true;
  }

  rOutputStream OutputStream::putByte(unsigned char c)
  {
    checkFD_();
    // FIXME: bufferize
    size_t size = write(fd_, &c, 1);
    assert_eq(size, 1);
    (void)size;
    return this;
  }

  void OutputStream::flush()
  {
    checkFD_();
    // FIXME: nothing since not bufferized for now
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

  void
  OutputStream::close()
  {
    checkFD_();
    if (::close(fd_))
      RAISE(libport::strerror(errno));
    fd_ = -1;
  }

  /*--------------.
  | Urbi bindings |
  `--------------*/

  rObject
  OutputStream::proto_make()
  {
    return new OutputStream(STDOUT_FILENO, false);
  }

  void
  OutputStream::initialize(object::CxxObject::Binder<object::OutputStream>& bind)
  {
    bind(SYMBOL(LT_LT), &OutputStream::put    );
    bind(SYMBOL(close), &OutputStream::close  );
    bind(SYMBOL(flush), &OutputStream::flush  );
    bind(SYMBOL(init),  &OutputStream::init   );
    bind(SYMBOL(put),   &OutputStream::putByte);
  }

  URBI_CXX_OBJECT_REGISTER(OutputStream);
}

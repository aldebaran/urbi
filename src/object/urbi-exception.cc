#include <object/object.hh>
#include <object/string.hh>
#include <object/symbols.hh>
#include <object/urbi-exception.hh>

namespace std
{

  /*------------.
  | call_type.  |
  `------------*/

  ostream&
  operator<<(ostream& o, const object::call_type& c)
  {
    if (c.second)
      o << *c.second << ": ";
    return o << c.first;
  }

  /*------------------.
  | call_stack_type.  |
  `------------------*/

  ostream&
  operator<<(ostream& o, const object::call_stack_type& cs)
  {
    rforeach (const object::call_type& c, cs)
      o << "    called from: " << c << endl;
    return o;
  }

}

namespace object
{

  /*------------------.
  | Urbi::Exception.  |
  `------------------*/

  std::ostream&
  UrbiException::dump(std::ostream& o) const
  {
    return
      o << "UrbiException"
        << "{"
        << libport::incendl
        <<   value_get() << "," << libport::iendl
        <<   backtrace_get()
        << libport::decendl
        << "}";
  }

  std::ostream&
  operator<<(std::ostream& o, const UrbiException& e)
  {
    return e.dump(o);
  }

}

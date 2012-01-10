/*
 * Copyright (C) 2007-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <sstream>
#include <urbi/object/object.hh>
#include <urbi/object/string.hh>
#include <urbi/object/symbols.hh>
#include <urbi/object/urbi-exception.hh>

namespace std
{

  /*------------.
  | call_type.  |
  `------------*/

  ostream&
  operator<<(ostream& o, const urbi::object::call_type& c)
  {
    if (c.second)
      o << *c.second << ": ";
    return o << c.first;
  }

  /*------------------.
  | call_stack_type.  |
  `------------------*/

  ostream&
  operator<<(ostream& o, const urbi::object::call_stack_type& cs)
  {
    rforeach (const urbi::object::call_type& c, cs)
      o << "    called from: " << c << endl;
    return o;
  }

}

namespace urbi
{
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
          <<   value_get()->call(SYMBOL(asPrintable))->as<String>()->value_get()
          << "," << libport::iendl
          <<   backtrace_get()
          << libport::decendl
          << "}";
    }

    const char*
    UrbiException::what() const throw()
    {
      std::stringstream s;
      dump(s);
      // Leaking, but calling this method means the program will terminate.
      return strdup(s.str().c_str());
    }

    std::ostream&
    operator<<(std::ostream& o, const UrbiException& e)
    {
      return e.dump(o);
    }

  }
}

/**
 ** \file object/code-class.cc
 ** \brief Creation of the URBI object code.
 */

#include <boost/lexical_cast.hpp>

#include "object/code-class.hh"

#include "kernel/uconnection.hh"

#include "object/atom.hh"
#include "object/object.hh"

namespace object
{
  rObject code_class;

  /*------------------.
  | Code primitives.  |
  `------------------*/

  rObject
  code_class_echo (rContext c, objects_type args)
  {
    // FIXME: First argument is ignored.
    c->value_get().connection.send
      (boost::lexical_cast<std::string>(*args[1]).c_str());
    return args[0];
  }


  void
  code_class_initialize ()
  {
#define DECLARE(Name)							\
      context_class->slot_set (#Name,					\
			       new Primitive(code_class_ ## Name));
      DECLARE(echo);
#undef DECLARE
  }

}; // namespace object

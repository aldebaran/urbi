/**
 ** \file object/call-class.cc
 ** \brief Creation of the URBI object call.
 */

#include <sstream>

#include <boost/format.hpp>
#include <boost/numeric/conversion/converter.hpp>

#include <libport/foreach.hh>
#include <libport/ufloat.hh>

#include "object/call-class.hh"

#include "object/alien.hh"
#include "object/atom.hh"
#include "object/object.hh"

#include "runner/runner.hh"

namespace object
{
  rObject call_class;

  /*------------------.
  | Call primitives.  |
  `------------------*/

  static const ast::exps_type&
  args_get (const rObject& self)
  {
    const rObject& alien_args = self->slot_get (SYMBOL(args));
    return unbox (const ast::exps_type&, alien_args);
  }

  static rObject
  context_get (const rObject& self)
  {
    return self->slot_get (SYMBOL(context));
  }

  static ast::Exp&
  arg_at (const rObject& f, ufloat arg_n, const libport::Symbol& func)
  {
    const ast::exps_type& func_args = args_get (f);

    int n;
    try {
      n = libport::ufloat_to_int (arg_n);
    }
    catch (boost::numeric::bad_numeric_cast& e)
    {
      throw BadInteger (arg_n, func);
    }

    if (n < 0 || n >= static_cast<int>(func_args.size ()))
      throw PrimitiveError (func,
			    (boost::format ("bad argument %1%") % n) .str ());

    ast::exps_type::const_iterator i = func_args.begin();
    advance (i, n);
    return **i;
  }

  static rObject
  call_class_evalArgAt (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    FETCH_ARG (1, Float);

    const rObject& scope = context_get (args[0]);

    return r.eval_in_scope (scope,
			    arg_at (args[0],
				    arg1->value_get (),
				    SYMBOL (evalArgAt)));
  }

  static rObject
  call_class_argString (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    FETCH_ARG (1, Float);

    const ast::Exp& exp = arg_at (args[0],
				  arg1->value_get (),
				  SYMBOL (argString));
    std::ostringstream str;
    str << exp;
    return new String (libport::Symbol (str.str ()));
  }

  static rObject
  call_class_evalArgs (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);

    const rObject& scope = context_get (args[0]);
    const ast::exps_type& func_args = args_get (args[0]);

    std::list<rObject> res;
    ast::exps_type::const_iterator i = func_args.begin();
    ++i;
    ast::exps_type::const_iterator i_end = func_args.end();

    for (; i != i_end; ++i)
      res.push_back (r.eval_in_scope (scope, **i));

    return new List (res);
  }

  static rObject
  call_class_argsCount (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return new Float (args_get (args[0]) .size ());
  }

  void
  call_class_initialize ()
  {
#define DECLARE(Name)							\
    DECLARE_PRIMITIVE(call, Name)

    DECLARE (argString);
    DECLARE (evalArgAt);
    DECLARE (evalArgs);
    DECLARE (argsCount);
#undef DECLARE
  }

}; // namespace object

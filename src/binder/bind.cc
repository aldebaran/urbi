#include <ast/nary.hh>
#include <binder/bind.hh>
#include <binder/binder.hh>
#include <object/urbi-exception.hh>
#include <parser/parser-utils.hh>

#include <kernel/server-timer.hh>

namespace binder
{
  // FIXME: This interface is not good enough, we need a cleaner
  // scheme to store and raise static errors.
  ast::rNary
  bind(ast::rConstNary a)
  {
    TIMER_PUSH("bind");
    Binder bind;
    ast::rAst res;
    if (getenv("BIND"))
      LIBPORT_ECHO("Binding: " << *a);
    try
    {
      bind(a);
      res = bind.result_get();
    }
    catch (const object::ParserError& pe)
    {
      ast::rMessage msg =
	new ast::Message(pe.location_get(),
			 parser::message_format(pe.location_get(),
						pe.msg_get()),
			 "error");
      ast::rNary nary = new ast::Nary(pe.location_get());
      nary->push_back(msg);
      res = nary;
    }
    TIMER_POP("bind");
    return res.unsafe_cast<ast::Nary>();
  }

} // namespace binder

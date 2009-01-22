#include <ast/nary.hh>
#include <binder/bind.hh>
#include <binder/binder.hh>

#include <kernel/server-timer.hh>

namespace binder
{
  template <typename T>
  libport::intrusive_ptr<T>
  bind(libport::intrusive_ptr<T> a)
  {
    TIMER_PUSH("bind");
    Binder bind;
    ast::rAst res = ast::analyze(bind, a);
    TIMER_POP("bind");
    return res ? res.unchecked_cast<T>() : libport::intrusive_ptr<T>();
  }

#define INSTANTIATE(Type)                               \
 template libport::intrusive_ptr<Type> bind<Type>(libport::intrusive_ptr<Type>)
  INSTANTIATE(ast::Ast);
  INSTANTIATE(const ast::Ast);
  INSTANTIATE(ast::Nary);
  INSTANTIATE(const ast::Nary);
#undef INSTANTIATE

} // namespace binder

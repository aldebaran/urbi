#include <libport/finally.hh>

#include <ast/parametric-ast.hh>
#include <flower/flower.hh>
#include <object/urbi-exception.hh>
#include <parser/parse.hh>
#include <parser/parser-impl.hh>

namespace flower
{
  using ast::ParametricAst;
  using libport::Finally;
  using libport::scoped_set;

  Flower::Flower()
    : in_function_(false)
    , in_loop_(false)
  {}

  void
  Flower::visit(const ast::Break* b)
  {
    if (!in_loop_)
      errors_.error(b->location_get(), "break: outside a loop");

    result_ = parser::parse("loopBreakTag.stop")->ast_get();
    has_break_ = true;
  }

  void
  Flower::visit(const ast::Continue* c)
  {
    if (!in_loop_)
      errors_.error(c->location_get(), "continue: outside a loop");

    result_ = parser::parse("loopContinueTag.stop")->ast_get();
    has_continue_ = true;
  }

  static inline
  ast::rExp
  brk(ast::rExp e)
  {
    static ParametricAst brk(
      "var loopBreakTag = new Tag |"
      "loopBreakTag: %exp:1");

    return exp(brk % e);
  }

  static inline
  ast::rExp
  cont(ast::rExp e)
  {
    static ParametricAst cont(
      "var loopContinueTag = new Tag |"
      "loopContinueTag: %exp:1");

    return exp(cont % e);
  }

  void
  Flower::visit(const ast::While* code)
  {
    Finally finally;
    finally << scoped_set(in_loop_, true)
            << scoped_set(has_break_, false)
            << scoped_set(has_continue_, false);

    ast::rExp test = recurse(code->test_get());
    ast::rExp body = recurse(code->body_get()->body_get());

    static ParametricAst whle(
      "while (%exp:1) %exp:2");

    ast::rExp res = body;

    if (has_continue_)
      res = cont(res.get());
    res = exp(whle % test % res);
    res.unchecked_cast<ast::While>()->flavor_set(code->flavor_get());
    if (has_break_)
      res = brk(res);

    result_ = res;
  }

  void
  Flower::visit(const ast::Foreach* code)
  {
    Finally finally;
    finally << scoped_set(in_loop_, true)
            << scoped_set(has_break_, false)
            << scoped_set(has_continue_, false);

    ast::rExp list = recurse(code->list_get());
    ast::rExp body = recurse(code->body_get());

    static ParametricAst each("%exp:1 . %id:2 (%exp:3)");
    // 'fillme' is a placeholder filled later. Parametric ASTs can't
    // parametrize formal arguments for now.
    static ParametricAst closure("closure (fillme) {%exp:1}");

    each % list;

    switch (code->flavor_get())
    {
    case ast::flavor_none:
    case ast::flavor_semicolon:
    case ast::flavor_pipe:
      each % SYMBOL(each);
      break;
    case ast::flavor_and:
      each % SYMBOL(each_AMPERSAND);
      break;
    case ast::flavor_comma:
      each.clear();
      pabort("Invalid flavor for 'for ... in': " << code->flavor_get());
    }

    if (has_continue_)
      body = cont(body);

    ast::rClosure c = (closure % body).result<ast::Closure>();
    // Rename the 'fillme' closure formal argument
    c->formals_get()->front()->what_set(code->index_get()->what_get());
    each % c;

    if (has_break_)
      result_ = brk(exp(each));
    else
      result_ = exp(each);
  }

  void
  Flower::visit(const ast::Function* code)
  {
    Finally finally;
    finally << scoped_set(in_function_, true)
            << scoped_set(has_return_, false);
    super_type::visit(code);
    if (has_return_)
    {
      static ast::ParametricAst a("var returnTag = new Tag | "
                                  "returnTag: %exp:1");
      ast::rScope copy =
        result_.unsafe_cast<ast::Function>()->body_get();
      copy->body_set(exp(a % copy->body_get()));
    }
  }

  void
  Flower::visit(const ast::Return* ret)
  {
    if (!in_function_)
      errors_.error(ret->location_get(), "return: outside a function");

    ast::rExp e = ret->value_get();

    static ParametricAst simple("returnTag.stop");
    static ParametricAst valued("returnTag.stop(%exp:1)");

    if (e)
      result_ = exp(valued % e);
    else
      result_ = exp(simple);

    has_return_ = true;
  }

} // namespace flower

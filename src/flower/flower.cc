#include <libport/finally.hh>

#include <ast/cloner.hxx>
#include <ast/parametric-ast.hh>

#include <flower/flower.hh>

#include <parser/parse.hh>
#include <parser/parser-impl.hh>

#include <rewrite/rewrite.hh>

namespace flower
{
  using libport::Finally;
  using libport::scoped_set;

  Flower::Flower()
    : in_catch_(false)
    , in_function_(false)
    , in_loop_(false)
  {}

  void
  Flower::visit(const ast::Break* b)
  {
    if (!in_loop_)
      errors_.error(b->location_get(), "break: outside a loop");

    has_break_ = true;

    PARAMETRIC_AST(res, "'$loopBreakTag'.stop");
    result_ = exp(res);
    result_->original_set(b);
  }

  void
  Flower::visit(const ast::Continue* c)
  {
    if (!in_loop_)
      errors_.error(c->location_get(), "continue: outside a loop");

    has_continue_ = true;

    PARAMETRIC_AST(res, "'$loopContinueTag'.stop");
    result_ = exp(res);
    result_->original_set(c);
  }

  static inline
  ast::rExp
  brk(ast::rExp e)
  {
    PARAMETRIC_AST(brk,
      "{var '$loopBreakTag' = Tag.newFlowControl(\"loopBreakTag\") |"
      "'$loopBreakTag': %exp:1}");

    // FIXME: e is visited twice
    return rewrite::rewrite(ast::rConstExp(exp(brk % e)));
  }

  static inline
  ast::rExp
  cont(ast::rExp e)
  {
    PARAMETRIC_AST(cont,
      "{var '$loopContinueTag' = Tag.newFlowControl(\"loopContinueTag\") |"
      "'$loopContinueTag': %exp:1}");


    // FIXME: e is visited twice
    return rewrite::rewrite(ast::rConstExp(exp(cont % e)));
  }

  void
  Flower::visit(const ast::While* code)
  {
    Finally finally;
    finally << scoped_set(in_loop_, true)
            << scoped_set(has_break_, false)
            << scoped_set(has_continue_, false);


    ast::rExp res = code->body_get()->body_get();
    res = res ? recurse(res) : new ast::Noop(code->location_get(), 0);

    if (has_continue_)
      res = cont(res.get());

    PARAMETRIC_AST(whle, "while (%exp:1) %exp:2");
    res = exp(whle % recurse(code->test_get()) % res);
    res.unchecked_cast<ast::While>()->flavor_set(code->flavor_get());

    if (has_break_)
      res = brk(res);

    result_ = res;
    result_->original_set(code);
  }

  void
  Flower::visit(const ast::Foreach* code)
  {
    Finally finally;
    finally << scoped_set(in_loop_, true)
            << scoped_set(has_break_, false)
            << scoped_set(has_continue_, false);

    PARAMETRIC_AST(each, "%exp:1 . %id:2 (%exp:3)");
    ast::rExp target = recurse(code->list_get());

    ast::rExp body = recurse(code->body_get());
    if (has_continue_)
      body = cont(body);

    // 'fillme' is a placeholder filled later. Parametric ASTs can't
    // parametrize formal arguments for now.
    PARAMETRIC_AST(closure, "closure (fillme) {%exp:1}");
    ast::rClosure c = (closure % body).result<ast::Closure>();
    // Rename the 'fillme' closure formal argument
    c->formals_get()->front()->what_set(code->index_get()->what_get());

    each % target;
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
      errors_.error(code->location_get(), "invalid flavor: `for,'");
      return;
    }
    each % c;

    if (has_break_)
      result_ = brk(exp(each));
    else
      result_ = exp(each);

    result_->original_set(code);
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
      PARAMETRIC_AST(a, "var '$returnTag' = "
				  "Tag.newFlowControl(\"returnTag\") | "
                                  "'$returnTag': %exp:1");
      ast::rScope copy =
        result_.unsafe_cast<ast::Function>()->body_get();
      // FIXME: children are rewritten twice
      copy->body_set(
        rewrite::rewrite(ast::rConstExp(exp(a % copy->body_get()))));
    }
    result_->original_set(code);
  }

  void
  Flower::visit(const ast::Return* ret)
  {
    if (!in_function_)
      errors_.error(ret->location_get(), "return: outside a function");

    has_return_ = true;

    if (ast::rExp e = ret->value_get())
    {
      PARAMETRIC_AST(a, "'$returnTag'.stop(%exp:1)");
      result_ = exp(a % e);
    }
    else
    {
      PARAMETRIC_AST(a, "'$returnTag'.stop");
      result_ = exp(a);
    }

    result_->original_set(ret);
  }

  void
  Flower::visit(const ast::Try* code)
  {
    Finally finally(scoped_set(has_general_catch_, false));
    super_type::visit(code);
  }

  void
  Flower::visit(const ast::Catch* code)
  {
    if (has_general_catch_)
      errors_.error(code->location_get(),
		    "catch: exception already caught by a previous clause");
    has_general_catch_ = !code->match_get();

    Finally finally(scoped_set(in_catch_, true));
    super_type::visit(code);
  }

  void
  Flower::visit(const ast::Throw* code)
  {
    if (!code->value_get() && !in_catch_)
      errors_.error(code->location_get(),
		    "throw: argumentless throw outside of a catch block");
    super_type::visit(code);
  }

} // namespace flower

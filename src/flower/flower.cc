#include <libport/finally.hh>

#include <ast/cloner.hxx>
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

    has_break_ = true;

    static ParametricAst res("'$loopBreakTag'.stop");
    result_ = exp(res);
    result_->original_set(b);
  }

  void
  Flower::visit(const ast::Continue* c)
  {
    if (!in_loop_)
      errors_.error(c->location_get(), "continue: outside a loop");

    has_continue_ = true;

    static ParametricAst res("'$loopContinueTag'.stop");
    result_ = exp(res);
    result_->original_set(c);
  }

  static inline
  ast::rExp
  brk(ast::rExp e)
  {
    static ParametricAst brk(
      "{var '$loopBreakTag' = Tag.newFlowControl(\"loopBreakTag\") |"
      "'$loopBreakTag': %exp:1}");

    return exp(brk % e);
  }

  static inline
  ast::rExp
  cont(ast::rExp e)
  {
    static ParametricAst cont(
      "{var '$loopContinueTag' = Tag.newFlowControl(\"loopContinueTag\") |"
      "'$loopContinueTag': %exp:1}");

    return exp(cont % e);
  }

  // The expansion of a try/catch block looks as follow:
  //    try
  //    {
  //     computation
  //    }
  //    catch (Proto e1)
  //    {
  //      use(e1);
  //    }
  //    catch (e2)
  //    {
  //      use(e2);
  //    }
  // becomes
  //    var $previousHandlerTag = currentRunner.exceptionHandlerTag |
  //    var $noExceptionTag = Tag.newFlowControl("noExceptionTag") |
  //    currentRunner.exceptionHandlerTag = Tag.newFlowControl("handlerTag") |
  //    $noExceptionTag: {
  //      var $exception = {
  //        currentRunner.exceptionHandlerTag: {
  //          var $res = computation |
  //          /* No exception occurred, restore handler */
  //          currentRunner.exceptionHandlerTag = $previousHandlerTag |
  //          $noExceptionTag.stop($res)
  //        }
  //      }
  //      /* An exception occurred, restore handler and handle it */
  //      currentRunner.exceptionHandlerTag = $previousHandlerTag |
  //      if ($exception.isA(Proto)) {
  //        var e1 = $exception |
  //        $noExceptionTag.stop(use(e1))
  //      } |
  //      var e2 = $exception |
  //      use(e2) |
  //      throw $exception
  //    }
  //
  // The expansion of
  //    throw(exc)
  // gives
  //    currentRunner.exceptionHandlerTag.stop(exc)

  void
  Flower::visit(const ast::Try* n)
  {
    if (n->handlers_get().empty())
      errors_.error(n->location_get(), "invalid: `try' without 'catch'");

    Finally finally;
    finally << scoped_set(catch_all_, 0U);

    ast::rNary new_handlers = new ast::Nary;
    foreach(const ast::rConstExp e, n->handlers_get())
      new_handlers->push_back(recurse(e));

    static ParametricAst try_block
      ("var '$previousHandlerTag' = currentRunner.exceptionHandlerTag |"
       "var '$noExceptionTag' = Tag.newFlowControl(\"noExceptionTag\") |"
       "currentRunner.exceptionHandlerTag = Tag.newFlowControl(\"handlerTag\") |"
       "'$noExceptionTag': {"
       "  var '$exception' = {"
       "    currentRunner.exceptionHandlerTag: {"
       "      var '$res' = %exp:1 |"
       "      currentRunner.exceptionHandlerTag = '$previousHandlerTag' |"
       "      '$noExceptionTag'.stop('$res')"
       "    }"
       "  } |"
       "  currentRunner.exceptionHandlerTag = '$previousHandlerTag' |"
       "  %exp:2 |"
       "  currentRunner.exceptionHandlerTag.stop('$exception')"
       "}");

    result_ = exp(try_block % recurse(n->body_get()) % new_handlers);
    result_->original_set(n);
  }

  void
  Flower::visit(const ast::Catch* c)
  {
    if (catch_all_)
      errors_.error(c->location_get(), "`catch' clause after catch-all clause");

    if (!c->match_get())
      ++catch_all_;

    static ParametricAst body("var %id:1 = '$exception' | "
			      "'$noExceptionTag'.stop(%exp:2)");
    ast::rExp res = exp(body % c->name_get() % recurse(c->body_get()));

    if (c->match_get())
    {
      static ParametricAst test("if ('$exception'.isA(%exp:1)) %exp:2");
      res = exp(test % recurse(c->match_get()) % res);
    }

    result_ = res;
    result_->original_set(c);
  }

  void
  Flower::visit(const ast::Throw* t)
  {
    static ParametricAst res("currentRunner.exceptionHandlerTag.stop(%exp:1)");
    result_ = exp(res % recurse(t->value_get()));
    result_->original_set(t);
  }

  void
  Flower::visit(const ast::While* code)
  {
    Finally finally;
    finally << scoped_set(in_loop_, true)
            << scoped_set(has_break_, false)
            << scoped_set(has_continue_, false);

    ast::rExp res = recurse(code->body_get()->body_get());

    if (has_continue_)
      res = cont(res.get());

    static ParametricAst whle("while (%exp:1) %exp:2");
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

    static ParametricAst each("%exp:1 . %id:2 (%exp:3)");
    each % recurse(code->list_get());

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

    ast::rExp body = recurse(code->body_get());
    if (has_continue_)
      body = cont(body);

    // 'fillme' is a placeholder filled later. Parametric ASTs can't
    // parametrize formal arguments for now.
    static ParametricAst closure("closure (fillme) {%exp:1}");
    ast::rClosure c = (closure % body).result<ast::Closure>();
    // Rename the 'fillme' closure formal argument
    c->formals_get()->front()->what_set(code->index_get()->what_get());
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
      static ast::ParametricAst a("var '$returnTag' = "
				  "Tag.newFlowControl(\"returnTag\") | "
                                  "'$returnTag': %exp:1");
      ast::rScope copy =
        result_.unsafe_cast<ast::Function>()->body_get();
      copy->body_set(exp(a % copy->body_get()));
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
      static ParametricAst a("'$returnTag'.stop(%exp:1)");
      result_ = exp(a % e);
    }
    else
    {
      static ParametricAst a("'$returnTag'.stop");
      result_ = exp(a);
    }

    result_->original_set(ret);
  }

} // namespace flower

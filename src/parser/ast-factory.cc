#include <ast/all.hh>
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>
#include <object/symbols.hh>
#include <parser/ast-factory.hh>
#include <parser/event-match.hh>
#include <parser/parser-impl.hh>
#include <parser/parse-result.hh>
#include <parser/parse.hh>
#include <rewrite/pattern-binder.hh>
#include <rewrite/rewrite.hh>

namespace std
{
  ostream&
  operator<< (ostream& o, const parser::case_type& c)
  {
    return o << "/* " << (const void*) &c << " */ "
             << "case "
             << libport::deref << c.first
             << " => "
             << libport::deref << c.second;
  }

  ostream&
  operator<< (ostream& o, const parser::cases_type& cs)
  {
    o << "/* " << (const void*) &cs << " */ "
      << "{" << endl;
    foreach (const parser::case_type& c, cs)
      o << "  " << c << endl;
    return o << "}";
  }

  ostream&
  operator<<(ostream& o, const parser::modifier_type& m)
  {
    return o << m.first << ": " << m.second;
  }

  ostream&
  operator<<(ostream& o, const parser::formals_type& f)
  {
    foreach (const parser::formal_type& var, f)
      o << var.first << " " << var.second;
    return o;
  }
}

namespace
{
  static ast::local_declarations_type*
  symbols_to_decs(const ast::loc& loc, parser::formals_type* formals)
  {
    if (!formals)
      return 0;
    ast::local_declarations_type* res = new ast::local_declarations_type();
    foreach (const parser::formal_type& var, *formals)
      res->push_back(new ast::LocalDeclaration(loc, var.first, var.second));
    delete formals;
    return res;
  }
}

namespace parser
{
  ast::rExp
  ast_at(const yy::location& loc,
         ast::rExp cond,
         ast::rExp body, ast::rExp onleave,
         ast::rExp duration)
  {
    if (!onleave)
      onleave = new ast::Noop(loc, 0);

    if (duration)
    {
      PARAMETRIC_AST(desugar,
                     "var '$at' = persist(%exp:1, %exp:2) |"
                     "at ('$at') %exp:3 onleave %exp:4");

      return exp(desugar % cond % duration % body % onleave);
    }
    else
    {
      PARAMETRIC_AST(desugar,
        "Control.at_(%exp:1, detach(%exp:2), detach(%exp:3))");

      return exp(desugar % cond % body % onleave);
    }
  }

  ast::rExp
  ast_event_catcher(const ast::loc& loc,
                    EventMatch& event,
                    ast::rExp body, ast::rExp onleave)
  {
    if (onleave)
    {
      PARAMETRIC_AST(a, "%exp:1 | %exp:2");
      body = exp(a % body % onleave);
    }

    if (event.guard)
    {
      PARAMETRIC_AST(desugar_guard,
                     "if (%exp:1) %exp:2;");
      body = exp(desugar_guard % event.guard % body);
    }

    if (event.pattern)
    {
      ast::rExp pattern = new ast::List(loc, event.pattern);
      rewrite::PatternBinder bind(ast_call(loc, SYMBOL(DOLLAR_pattern)), loc);
      bind(pattern.get());
      PARAMETRIC_AST(desugar,
                     "detach("
                     "{"
                     "  %exp:1.onEvent("
                     "  closure ('$evt')"
                     "  {"
                     "    var '$pattern' = Pattern.new(%exp:2) |"
                     "    if ('$pattern'.match('$evt'.payload))"
                     "    {"
                     "      %exp: 3 |"
                     "      %exp: 4 |"
                     "    }"
                     "  })"
                     "})");
      return exp(desugar
                 % event.event
                 % bind.result_get().unchecked_cast<ast::Exp>()
                 % bind.bindings_get()
                 % body);
    }
    else
    {
      PARAMETRIC_AST(desugar_no_pattern,
                     "detach("
                     "{"
                     "  %exp:1.onEvent(closure ('$evt')"
                     "  {"
                     "    %exp: 2 |"
                     "  })"
                     "})");
      return exp(desugar_no_pattern % event.event % body);
    }
  }


  ast::rExp
  ast_at_event(const ast::loc& loc,
               EventMatch& event,
               ast::rExp body, ast::rExp onleave)
  {
    PARAMETRIC_AST(desugar_body,
                   "%exp:1 |"
                   "waituntil(!'$evt'.active)");
    body = exp(desugar_body % body);
    return ast_event_catcher(loc, event, body, onleave);
  }


  /// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
  /// \param op must be & or |.
  ast::rExp
  ast_bin(const yy::location& l,
          ast::flavor_type op, ast::rExp lhs, ast::rExp rhs)
  {
    ast::rExp res = 0;
    assert (lhs);
    assert (rhs);
    switch (op)
    {
    case ast::flavor_pipe:
    {
      ast::rPipe pipe;
      if (pipe = lhs.unsafe_cast<ast::Pipe>())
        pipe->children_get().push_back(rhs);
      else
      {
        pipe = new ast::Pipe(l, ast::exps_type());
        pipe->children_get().push_back(lhs);
        pipe->children_get().push_back(rhs);
      }
      res = pipe;
      break;
    }
    case ast::flavor_and:
    {
      ast::rAnd rand;
      if (rand = lhs.unsafe_cast<ast::And>())
        rand->children_get().push_back(rhs);
      else
      {
        rand = new ast::And(l, ast::exps_type());
        rand->children_get().push_back(lhs);
        rand->children_get().push_back(rhs);
      }
      res = rand;
      break;
    }
    case ast::flavor_comma:
    case ast::flavor_semicolon:
    {
      ast::rNary nary = new ast::Nary(l);
      nary->push_back(lhs, op);
      nary->push_back(rhs);
      res = nary;
      break;
    }
    case ast::flavor_none:
      pabort(op);
    }
    return res;
  }


  /// "<method> (args)".
  ast::rCall
  ast_call(const yy::location& l,
           libport::Symbol method)
  {
    return ast_call(l, new ast::Implicit(l), method);
  }


  /// "<target> . <method> (args)".
  ast::rCall
  ast_call(const yy::location& l,
           ast::rExp target, libport::Symbol method, ast::exps_type* args)
  {
    return new ast::Call(l, args, target, method);
  }

  /// "<target> . <method> ()".
  ast::rCall
  ast_call(const yy::location& l, ast::rExp target, libport::Symbol method)
  {
    return ast_call(l, target, method, 0);
  }

  /// "<target> . <method> (<arg1>)".
  ast::rCall
  ast_call(const yy::location& l,
           ast::rExp target, libport::Symbol method, ast::rExp arg1)
  {
    ast::rCall res = ast_call(l, target, method, new ast::exps_type);
    res->arguments_get()->push_back(arg1);
    return res;
  }


  /// "<target> . <method> (<arg1>, <arg2>)".
  /// "<target> . <method> (<arg1>, <arg2>, <arg3>)".
  ast::rCall
  ast_call(const yy::location& l,
           ast::rExp target, libport::Symbol method,
           ast::rExp arg1, ast::rExp arg2, ast::rExp arg3)
  {
    ast::rCall res = ast_call(l, target, method, new ast::exps_type);
    res->arguments_get()->push_back(arg1);
    res->arguments_get()->push_back(arg2);
    if (arg3)
      res->arguments_get()->push_back(arg3);
    return res;
  }


  ast::rExp
  ast_closure(ast::rExp value)
  {
    PARAMETRIC_AST(a, "closure () { %exp:1 }");
    return exp(a % value);
  }

  ast::rExp
  ast_every(const yy::location&, ast::flavor_type flavor,
            ast::rExp test, ast::rExp body)
  {
    // every (exp:1) exp:2.
    PARAMETRIC_AST(semi,
    "detach ({\n"
    "  var deadline = shiftedTime |\n"
    "  var controlTag = Tag.newFlowControl |\n"
    "  throw\n"
    "  {\n"
    "    controlTag: loop\n"
    "    {\n"
    "      detach ({ try { %exp:2 } catch (var e) { controlTag.stop(e)} }) |\n"
    "      deadline += %exp:1 |\n"
    "      sleep (deadline - shiftedTime)\n"
    "    };\n"
    "  }\n"
    "})\n");

    // every| (exp:1) exp:2.
    PARAMETRIC_AST(pipe,
    "detach ({\n"
    "  for (var deadline = shiftedTime; true;\n"
    "       deadline = Control.'every|sleep'(deadline, %exp:1))\n"
    "    %exp:2\n"
    "})\n");

    return exp((flavor == ast::flavor_semicolon ? semi : pipe)
               % test % body);
  }


  // Build a C-like for loop.
  ast::rExp
  ast_for(const yy::location&, ast::flavor_type flavor,
          ast::rExp init, ast::rExp test, ast::rExp inc,
          ast::rExp body)
  {
    // The increment is included directly in the condition to make
    // sure it is executed on `continue'.

    PARAMETRIC_AST(pipe,
      "{"
      "  %exp:1 |"
      "  var '$first' = true |"
      "  while| ({ if ('$first') '$first' = false else %exp:2|"
      "            %exp:3})"
      "    %exp:4"
      "}"
      );

    // Don't use ";" for costs that should not be visible to the user:
    // $first.
    PARAMETRIC_AST(semi,
      "{"
      "  %exp:1;"
      "  var '$first' = true |"
      "  while ({ if ('$first') '$first' = false else %exp:2|"
      "           %exp:3})"
      "    %exp:4"
      "}"
      );
    return exp((flavor == ast::flavor_semicolon ? semi : pipe)
               % init % inc % test % body);
  }


  ast::rExp
  ast_if(const yy::location& l,
         ast::rExp cond, ast::rExp iftrue, ast::rExp iffalse)
  {
    return new ast::If(l, ast_strip(cond),
                       ast_scope(l, iftrue),
		       iffalse ? ast_scope(l, iffalse) : new ast::Noop(l, 0));
  }


  ast::rLValue
  ast_lvalue_once(const ast::rLValue& lvalue)
  {
    ast::rCall tmp = ast_call(lvalue->location_get(), SYMBOL(DOLLAR_tmp));

    if (lvalue->call()->target_implicit())
      return lvalue.get();
    else
      return ast_call(lvalue->location_get(), tmp, lvalue->call()->name_get());
  }

  ast::rExp
  ast_lvalue_wrap(const ast::rLValue& lvalue, const ast::rExp& e)
  {
    PARAMETRIC_AST(wrap,
                   "{"
                   "var '$tmp' = %exp:1;"
                   "%exp:2;"
                   "}"
      );

    if (lvalue->call()->target_implicit())
      return e;
    else
    {
      wrap % lvalue->call()->target_get() % e;
      return exp(wrap);
    }
  }

  ast::rExp
  ast_nil()
  {
    PARAMETRIC_AST(nil, "nil");
    return exp(nil);
  }

  ast::rRoutine
  ast_routine(ParserImpl& up,
              const ast::loc& loc, bool closure,
              const ast::loc& floc, formals_type* f,
              const ast::loc& bloc, ast::rExp b)
  {
    if (closure && !f)
      up.error(loc, "closure cannot be lazy");
    // Yet we might build one...
    return new ast::Routine(loc, closure,
                            symbols_to_decs(floc, f),
                            ast_scope(bloc, b));
  }


  /// Return \a e in a ast::Scope unless it is already one.
  ast::rScope
  ast_scope(const yy::location& l, ast::rExp target, ast::rExp e)
  {
    if (ast::rScope res = e.unsafe_cast<ast::Scope>())
      return res;
    else if (target)
      return new ast::Do(l, e, target);
    else
      return new ast::Scope(l, e);
  }

  ast::rScope
  ast_scope(const yy::location& l, ast::rExp e)
  {
    return ast_scope(l, 0, e);
  }

  ast::rExp
  ast_string(const yy::location& l, libport::Symbol s)
  {
    return new ast::String(l, s);
  }


  /*------------.
  | ast_strip.  |
  `------------*/

  ast::rExp
  ast_strip(ast::rNary nary)
  {
    ast::rExp res = nary;
    // Remove useless nary and statement if there's only one child.
    if (nary->children_get().size() == 1)
      res = (nary->children_get().front()
             .unchecked_cast<ast::Stmt>()
             ->expression_get());
    return res;
  }

  ast::rExp
  ast_strip(ast::rExp e)
  {
    if (ast::rNary nary = e.unsafe_cast<ast::Nary>())
      return ast_strip(nary);
    else
      return e;
  }


  ast::rExp
  ast_switch(const yy::location&, ast::rExp cond,
             const cases_type& cases, ast::rExp def)
  {
    const ast::loc& loc = cond->location_get();
    ast::rExp inner = def ? def : ast_nil();
    rforeach (const case_type& c, cases)
    {
      PARAMETRIC_AST(desugar,
                     "var '$pattern' = Pattern.new(%exp:1) |"
                     "if (if ('$pattern'.match('$switch'))"
                     "    {"
                     "      %exp:2 |"
                     "      %exp:3"
                     "    }"
                     "    else"
                     "      false)"
                     "{"
                     "  %exp:4 |"
                     "  %exp:5"
                     "}"
                     "else"
                     "  %exp:6"
        );

      PARAMETRIC_AST(cond,
                     "true");

      ast::rExp condition = c.first->guard_get();
      if (!condition)
        condition = exp(cond);

      rewrite::PatternBinder bind(ast_call(loc, SYMBOL(DOLLAR_pattern)), loc);
      bind(c.first->pattern_get().get());

      desugar
        % bind.result_get().unchecked_cast<ast::Exp>()
        % bind.bindings_get()
        % condition
        % bind.bindings_get()
        % c.second
        % inner;
      inner = ast::exp(desugar);
    }

    PARAMETRIC_AST(sw, "{ var '$switch' = %exp:1 | %exp:2 }");
    return exp(sw % cond % inner);
  }

  ast::rExp
  ast_timeout(const ast::rExp& duration, const ast::rExp& body)
  {
    PARAMETRIC_AST(desugar,
		   "{"
		   " var '$tag' = Tag.new |"
		   " '$tag':"
		   "   {"
		   "      {"
		   "        sleep(%exp:1) | '$tag'.stop"
		   "      },"
		   "     %exp:2 | '$tag'.stop"
		   "   }"
		   "}");
    return exp(desugar % duration % body);
  }


  ast::rExp
  ast_waituntil(const yy::location&,
                const ast::rExp& cond, ast::rExp duration)
  {
    if (duration)
    {
      PARAMETRIC_AST(desugar,
        "{"
        "  var '$waituntil' = persist(%exp:1, %exp:2) |"
        "  waituntil('$waituntil'())"
        "}"
        );
      return exp(desugar % cond % duration);
    }
    else
    {
      PARAMETRIC_AST(desugar,
                     "{var '$tag' = Tag.new |"
                     "'$tag': {at (%exp:1) '$tag'.stop | sleep(inf)}}");
      return exp(desugar % cond);
    }
  }


  ast::rExp
  ast_waituntil_event(const ast::loc& loc,
                      ast::rExp event,
                      ast::exps_type* payload)
  {
    if (!payload)
    {
      PARAMETRIC_AST(desugar, "%exp:1.'waituntil'(nil)");
      return exp(desugar % event);
    }

    PARAMETRIC_AST
      (desugar,
       "{"
       "  var '$pattern' = Pattern.new(%exp:1) |"
       "  %exp:2.'waituntil'('$pattern') |"
       "  {"
       "    %unscope: 2 |"
       "    %exp:3 |"
       "  }"
       "}");

    ast::rList d_payload = new ast::List(loc, payload);

    rewrite::PatternBinder bind(ast_call(loc, SYMBOL(DOLLAR_pattern)), loc);
    bind(d_payload.get());

    return exp(desugar
               % bind.result_get().unchecked_cast<ast::Exp>()
               % event
               % bind.bindings_get());
  }

  ast::rExp
  ast_whenever_event(const ast::loc& loc,
                     EventMatch& event,
                     ast::rExp body, ast::rExp onleave)
  {
    PARAMETRIC_AST(desugar_body,
                   "while (true)"
                   "{"
                   "  %exp:1 |"
                   "  if(!'$evt'.active)"
                   "    break"
                   "}");
    body = exp(desugar_body % body);
    return ast_event_catcher(loc, event, body, onleave);
  }

  ast::rExp
  ast_whenever(const yy::location&,
               ast::rExp cond,
               ast::rExp body, ast::rExp else_stmt,
               ast::rExp duration)
  {
    // FIXME: Be smarter on empty else_stmt.
    if (!else_stmt)
      else_stmt = ast_nil();
    if (duration)
    {
      PARAMETRIC_AST(desugar,
        "var '$whenever' = persist(%exp:1, %exp:2) |"
        "Control.whenever_('$whenever'.val, %exp:3, %exp:4) |'"
        );
      return exp(desugar % cond % duration % body % else_stmt);
    }
    else
    {
      PARAMETRIC_AST(desugar,
        "Control.whenever_(%exp:1, %exp:2, %exp:3)");
      return exp(desugar % cond % body % else_stmt);
    }
  }

}

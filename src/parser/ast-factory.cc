/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#include <libport/format.hh>

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
  operator<< (ostream& o, const parser::AstFactory::case_type& c)
  {
    return o << "/* " << (const void*) &c << " */ "
             << "case "
             << libport::deref << c.first
             << " => "
             << libport::deref << c.second;
  }

  ostream&
  operator<< (ostream& o, const parser::AstFactory::cases_type& cs)
  {
    o << "/* " << (const void*) &cs << " */ "
      << "{" << endl;
    foreach (const parser::AstFactory::case_type& c, cs)
      o << "  " << c << endl;
    return o << "}";
  }

  ostream&
  operator<<(ostream& o, const parser::AstFactory::modifier_type& m)
  {
    return o << m.first << ": " << m.second;
  }

  ostream&
  operator<<(ostream& o, const parser::AstFactory::formals_type& f)
  {
    foreach (const parser::AstFactory::formal_type& var, f)
      o << var.first << " " << var.second;
    return o;
  }
}

namespace
{
  static ast::local_declarations_type*
  symbols_to_decs(const ast::loc& loc,
                  parser::AstFactory::formals_type* formals)
  {
    if (!formals)
      return 0;
    ast::local_declarations_type* res = new ast::local_declarations_type();
    foreach (const parser::AstFactory::formal_type& var, *formals)
      res->push_back(new ast::LocalDeclaration(loc, var.first, var.second));
    delete formals;
    return res;
  }
}

namespace parser
{
  ast::rExp
  AstFactory::make_at(const location& loc,
                      ast::rExp cond,
                      ast::rExp body, ast::rExp onleave,
                      ast::rExp duration) // const
  {
    if (!onleave)
      onleave = new ast::Noop(loc, 0);

    if (duration)
    {
      PARAMETRIC_AST
        (desugar,
         "{"
         "  var '$at' = persist(%exp:1, %exp:2) |"
         "  at ('$at'()) %exp:3 onleave %exp:4"
         "}"
        );
      return exp(desugar % cond % duration % body % onleave);
    }
    else
    {
      PARAMETRIC_AST
        (desugar,
         "Control.at_(%exp:1, detach(%exp:2), detach(%exp:3))");

      return exp(desugar % cond % body % onleave);
    }
  }

  ast::rExp
  AstFactory::make_event_catcher(const location& loc,
                                 EventMatch& event,
                                 ast::rExp body, ast::rExp onleave) // const
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
      rewrite::PatternBinder
        bind(make_call(loc, SYMBOL(DOLLAR_pattern)), loc);
      bind(pattern.get());
      PARAMETRIC_AST
        (desugar,
         "detach(\n"
         "{\n"
         "  %exp:1.onEvent(\n"
         "  closure ('$evt')\n"
         "  {\n"
         "    var '$pattern' = Pattern.new(%exp:2) |\n"
         "    if ('$pattern'.match('$evt'.payload))\n"
         "    {\n"
         "      %exp: 3 |\n"
         "      %exp: 4 |\n"
         "    }\n"
         "  })\n"
         "})");
      return exp(desugar
                 % event.event
                 % bind.result_get().unchecked_cast<ast::Exp>()
                 % bind.bindings_get()
                 % body);
    }
    else
    {
      PARAMETRIC_AST
        (desugar_no_pattern,
         "detach(\n"
         "{\n"
         "  %exp:1.onEvent(closure ('$evt')\n"
         "  {\n"
         "    %exp: 2 |\n"
         "  })\n"
         "})\n");
      return exp(desugar_no_pattern % event.event % body);
    }
  }


  ast::rExp
  AstFactory::make_at_event(const location& loc,
                            EventMatch& event,
                            ast::rExp body, ast::rExp onleave) // const
  {
    PARAMETRIC_AST
      (desugar_body,
       "%exp:1 |"
       "waituntil(!'$evt'.active)");
    body = exp(desugar_body % body);
    return make_event_catcher(loc, event, body, onleave);
  }


  ast::rExp
  AstFactory::make_bin(const location& l,
                       ast::flavor_type op,
                       ast::rExp lhs, ast::rExp rhs) // const
  {
    assert(lhs);
    assert(rhs);
    ast::rExp res = 0;
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


  ast::rBinding
  AstFactory::make_binding(const location& l, const
                           bool constant,
                           const location& exp_loc, ast::rExp exp)
  {
    if (false
        // Must be an LValue,
        || !exp.unsafe_cast<ast::LValue>()
        // But must not be one of the subclasses about properties.
        || exp.unsafe_cast<ast::PropertyAction>()
        // Or can be a call without argument, i.e., "Foo.bar".
        || (exp.unsafe_cast<ast::Call>()
            && exp.unsafe_cast<ast::Call>()->arguments_get()))
      throw syntax_error(exp_loc,
                         libport::format("syntax error, "
                                         "%s is not a valid lvalue",
                                         *exp));

    ast::rBinding res = new ast::Binding(l, exp.unchecked_cast<ast::LValue>());
    res->constant_set(constant);
    return res;
  }

  ast::rCall
  AstFactory::make_call(const location& l,
                        libport::Symbol method) // const
  {
    return make_call(l, new ast::Implicit(l), method);
  }


  ast::rCall
  AstFactory::make_call(const location& l,
                        ast::rExp target, libport::Symbol method, ast::exps_type* args) // const
  {
    return new ast::Call(l, args, target, method);
  }

  /// "<target> . <method> ()".
  ast::rCall
  AstFactory::make_call(const location& l,
                        ast::rExp target, libport::Symbol method) // const
  {
    return make_call(l, target, method, 0);
  }

  ast::rCall
  AstFactory::make_call(const location& l,
                        ast::rExp target,
                        libport::Symbol method, ast::rExp arg1) // const
  {
    ast::rCall res = make_call(l, target, method, new ast::exps_type);
    res->arguments_get()->push_back(arg1);
    return res;
  }


  ast::rCall
  AstFactory::make_call(const location& l,
                        ast::rExp target, libport::Symbol method,
                        ast::rExp arg1, ast::rExp arg2, ast::rExp arg3) // const
  {
    ast::rCall res = make_call(l, target, method, new ast::exps_type);
    res->arguments_get()->push_back(arg1);
    res->arguments_get()->push_back(arg2);
    if (arg3)
      res->arguments_get()->push_back(arg3);
    return res;
  }


  ast::rExp
  AstFactory::make_closure(ast::rExp value) // const
  {
    PARAMETRIC_AST(a, "closure () { %exp:1 }");
    return exp(a % value);
  }

  ast::rExp
  AstFactory::make_every(const location&, ast::flavor_type flavor,
                         ast::rExp test, ast::rExp body) // const
  {
    // every (exp:1) exp:2.
    PARAMETRIC_AST
      (semi,
       "controlTag:\n"
       "for (var deadline = shiftedTime; true;\n"
       "     deadline = Control.'every;sleep'(deadline, %exp:1))\n"
       "  detach ({ try { %exp:2 } catch (var e) { controlTag.stop(e)} })\n");

    // every| (exp:1) exp:2.
    PARAMETRIC_AST
      (pipe,
       "for (var deadline = shiftedTime; true;\n"
       "     deadline = Control.'every|sleep'(deadline, %exp:1))\n"
       "  %exp:2\n");

    return exp((flavor == ast::flavor_semicolon ? semi : pipe)
               % test % body);
  }


  // Build a for(iterable) loop.
  ast::rExp
  AstFactory::make_for(const location&, ast::flavor_type flavor,
                       ast::rExp iterable, ast::rExp body) // const
  {
    PARAMETRIC_AST
      (ampersand,
       "for&(var '$for': %exp:1)\n"
       "  %exp:2\n"
        );
    PARAMETRIC_AST
      (pipe,
       "for|(var '$for': %exp:1)\n"
       "  %exp:2\n"
        );
    PARAMETRIC_AST
      (semi,
       "for (var '$for': %exp:1)\n"
       "  %exp:2"
        );
    return exp((flavor == ast::flavor_and ? ampersand
                : flavor == ast::flavor_pipe ? pipe
                : semi )
               % iterable % body);
  }


  // Build a for(var id : iterable) loop.
  ast::rExp
  AstFactory::make_for(const location& l, ast::flavor_type flavor,
                       const location& id_loc, libport::Symbol id,
                       ast::rExp iterable, ast::rExp body) // const
  {
    return
      new ast::Foreach(l, flavor,
                       new ast::LocalDeclaration(id_loc, id,
                                                 new ast::Implicit(id_loc)),
                       iterable, make_scope(l, body));
  }


  // Build a C-like for loop.
  ast::rExp
  AstFactory::make_for(const location&, ast::flavor_type flavor,
                       ast::rExp init, ast::rExp test, ast::rExp inc,
                       ast::rExp body) // const
  {
    // The increment is included directly in the condition to make
    // sure it is executed on `continue'.

    PARAMETRIC_AST
      (pipe,
       "{\n"
       "  %exp:1 |\n"
       "  var '$first' = true |\n"
       "  while| ({ if ('$first') '$first' = false else %exp:2|\n"
       "            %exp:3})\n"
       "    %exp:4\n"
       "}");

    // Don't use ";" for costs that should not be visible to the user:
    // $first.
    PARAMETRIC_AST
      (semi,
       "{\n"
       "  %exp:1|\n" // When not entering the loop, we want 0 cycles consumed.
       "  var '$first' = true |\n"
       "  while ({ if ('$first') '$first' = false else %exp:2|\n"
       "           %exp:3})\n"
       "    %exp:4\n"
       "}");
    return exp((flavor == ast::flavor_semicolon ? semi : pipe)
               % init % inc % test % body);
  }


  ast::rExp
  AstFactory::make_if(const location& l,
                      ast::rExp cond, ast::rExp iftrue, ast::rExp iffalse) // const
  {
    return new ast::If(l, make_strip(cond),
                       make_scope(l, iftrue),
                       iffalse ? make_scope(l, iffalse) : new ast::Noop(l, 0));
  }


  ast::rLValue
  AstFactory::make_lvalue_once(const ast::rLValue& lvalue) // const
  {
    ast::rCall tmp = make_call(lvalue->location_get(), SYMBOL(DOLLAR_tmp));

    if (lvalue->call()->target_implicit())
      return lvalue.get();
    else
      return make_call(lvalue->location_get(), tmp, lvalue->call()->name_get());
  }

  ast::rExp
  AstFactory::make_lvalue_wrap(const ast::rLValue& lvalue,
                               const ast::rExp& e) // const
  {
    PARAMETRIC_AST
      (wrap,
       "{\n"
       "  var '$tmp' = %exp:1;\n"
       "  %exp:2;\n"
       "}\n");

    if (lvalue->call()->target_implicit())
      return e;
    else
    {
      wrap % lvalue->call()->target_get() % e;
      return exp(wrap);
    }
  }

  ast::rExp
  AstFactory::make_nil() // const
  {
    PARAMETRIC_AST(nil, "nil");
    return exp(nil);
  }

  ast::rRoutine
  AstFactory::make_routine(const location& loc, bool closure,
                           const location& floc, formals_type* f,
                           const location& bloc, ast::rExp b) // const
  {
    if (closure && !f)
      throw syntax_error(loc, "closure cannot be lazy");
    return new ast::Routine(loc, closure,
                            symbols_to_decs(floc, f),
                            make_scope(bloc, b));
  }


  /// Return \a e in a ast::Scope unless it is already one.
  ast::rScope
  AstFactory::make_scope(const location& l,
                         ast::rExp target, ast::rExp e) // const
  {
    if (ast::rScope res = e.unsafe_cast<ast::Scope>())
      return res;
    else if (target)
      return new ast::Do(l, e, target);
    else
      return new ast::Scope(l, e);
  }

  ast::rScope
  AstFactory::make_scope(const location& l, ast::rExp e) // const
  {
    return make_scope(l, 0, e);
  }

  ast::rExp
  AstFactory::make_string(const location& l, libport::Symbol s) // const
  {
    return new ast::String(l, s);
  }


  /*-------------.
  | make_strip.  |
  `-------------*/

  ast::rExp
  AstFactory::make_strip(ast::rNary nary) // const
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
  AstFactory::make_strip(ast::rExp e) // const
  {
    if (ast::rNary nary = e.unsafe_cast<ast::Nary>())
      return make_strip(nary);
    else
      return e;
  }


  ast::rExp
  AstFactory::make_switch(const location&, ast::rExp cond,
                          const cases_type& cases, ast::rExp def) // const
  {
    const location& loc = cond->location_get();
    ast::rExp inner = def ? def : make_nil();
    rforeach (const case_type& c, cases)
    {
      PARAMETRIC_AST
        (desugar,
         "var '$pattern' = Pattern.new(%exp:1) |\n"
         "if (if ('$pattern'.match('$switch'))\n"
         "    {\n"
         "      %exp:2 |\n"
         "      %exp:3\n"
         "    }\n"
         "    else\n"
         "      false)\n"
         "{\n"
         "  %exp:4 |\n"
         "  %exp:5\n"
         "}\n"
         "else\n"
         "  %exp:6\n"
        );

      PARAMETRIC_AST(cond,
                     "true");

      ast::rExp condition = c.first->guard_get();
      if (!condition)
        condition = exp(cond);

      rewrite::PatternBinder bind(make_call(loc, SYMBOL(DOLLAR_pattern)), loc);
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
  AstFactory::make_timeout(const ast::rExp& duration,
                           const ast::rExp& body) // const
  {
    PARAMETRIC_AST
      (desugar,
       "{\n"
       " var '$tag' = Tag.new |\n"
       " '$tag':\n"
       "   {\n"
       "      {\n"
       "        sleep(%exp:1) | '$tag'.stop\n"
       "      },\n"
       "     %exp:2 | '$tag'.stop\n"
       "   }\n"
       "}");
    return exp(desugar % duration % body);
  }


  ast::rExp
  AstFactory::make_waituntil(const location&,
                             const ast::rExp& cond, ast::rExp duration) // const
  {
    if (duration)
    {
      PARAMETRIC_AST(desugar,
                     "{\n"
                     "  var '$waituntil' = persist(%exp:1, %exp:2) |\n"
                     "  waituntil('$waituntil'())\n"
                     "}\n"
        );
      return exp(desugar % cond % duration);
    }
    else
    {
      PARAMETRIC_AST
        (desugar,
         "{var '$tag' = Tag.new |\n"
         "'$tag': {at (%exp:1) '$tag'.stop | sleep(inf)}}");
      return exp(desugar % cond);
    }
  }


  ast::rExp
  AstFactory::make_waituntil_event(const location& loc,
                                   ast::rExp event,
                                   ast::exps_type* payload) // const
  {
    if (!payload)
    {
      PARAMETRIC_AST(desugar, "%exp:1.'waituntil'(nil)");
      return exp(desugar % event);
    }

    PARAMETRIC_AST
      (desugar,
       "{\n"
       "  var '$pattern' = Pattern.new(%exp:1) |\n"
       "  %exp:2.'waituntil'('$pattern') |\n"
       "  {\n"
       "    %unscope: 2 |\n"
       "    %exp:3 |\n"
       "  }\n"
       "}");

    ast::rList d_payload = new ast::List(loc, payload);

    rewrite::PatternBinder bind(make_call(loc, SYMBOL(DOLLAR_pattern)), loc);
    bind(d_payload.get());

    return exp(desugar
               % bind.result_get().unchecked_cast<ast::Exp>()
               % event
               % bind.bindings_get());
  }

  ast::rExp
  AstFactory::make_whenever_event(const location& loc,
                                  EventMatch& event,
                                  ast::rExp body, ast::rExp onleave) // const
  {
    PARAMETRIC_AST(desugar_body,
                   "while (true)\n"
                   "{\n"
                   "  %exp:1 |\n"
                   "  if(!'$evt'.active)\n"
                   "    break\n"
                   "}");
    body = exp(desugar_body % body);
    return make_event_catcher(loc, event, body, onleave);
  }

  ast::rExp
  AstFactory::make_whenever(const location&,
                            ast::rExp cond,
                            ast::rExp body, ast::rExp else_stmt,
                            ast::rExp duration) // const
  {
    // FIXME: Be smarter on empty else_stmt.
    if (!else_stmt)
      else_stmt = make_nil();
    if (duration)
    {
      PARAMETRIC_AST(desugar,
                     "var '$whenever' = persist(%exp:1, %exp:2) |\n"
                     "Control.whenever_('$whenever'.val, %exp:3, %exp:4) |'\n"
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

/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/deref.hh>
#include <libport/format.hh>
#include <libport/separate.hh>

#include <ast/all.hh>
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>
#include <object/symbols.hh>
#include <ast/factory.hh>
#include <ast/event-match.hh>
#include <parser/parser-impl.hh>
#include <parser/parse-result.hh>
#include <parser/parse.hh>
#include <rewrite/pattern-binder.hh>
#include <rewrite/rewrite.hh>

namespace std
{
  ostream&
  operator<< (ostream& o, const ast::Factory::case_type& c)
  {
    return o << "/* " << (const void*) &c << " */ "
             << "case "
             << ::libport::deref << c.first
             << " => "
             << ::libport::deref << c.second;
  }

  ostream&
  operator<< (ostream& o, const ast::Factory::cases_type& cs)
  {
    o << "/* " << (const void*) &cs << " */ "
      << "{" << endl;
    foreach (const ast::Factory::case_type& c, cs)
      o << "  " << c << endl;
    return o << "}";
  }

  ostream&
  operator<<(ostream& o, const ast::Factory::modifier_type& m)
  {
    return o << m.first << ": " << m.second;
  }

  ostream&
  operator<<(ostream& o, const ast::Factory::formals_type& f)
  {
    foreach (const ast::Factory::formal_type& var, f)
      o << var.first << " " << var.second;
    return o;
  }
}

#define SYNTAX_ERROR(Loc, ...)                   \
  throw yy::parser::syntax_error(Loc, libport::format(__VA_ARGS__))

#define FLAVOR_ERROR(Keyword)                                           \
  SYNTAX_ERROR(flavor_loc, "invalid flavor: %s%s", Keyword, flavor)

#define FLAVOR_IS(Flav1)                        \
  (flavor == flavor_ ## Flav1)

#define FLAVOR_IS2(Flav1, Flav2)                \
  (FLAVOR_IS(Flav1) || FLAVOR_IS(Flav2))

#define FLAVOR_IS3(Flav1, Flav2, Flav3)                \
  (FLAVOR_IS2(Flav1, Flav2) || FLAVOR_IS(Flav3))

#define FLAVOR_IS4(Flav1, Flav2, Flav3, Flav4)          \
  (FLAVOR_IS2(Flav1, Flav2, Flav3) || FLAVOR_IS(Flav4))

/// Generate a parse error for invalid keyword/flavor combination.
/// The check is performed by the parser, not the scanner, because
/// some keywords may, or may not, have some flavors dependencies
/// on the syntactic construct.  See the various "for"s for instance.
#define FLAVOR_CHECK(Keyword, Condition)        \
  do                                            \
    if (!(Condition))                           \
      FLAVOR_ERROR(Keyword);                    \
  while (0)

#define FLAVOR_CHECK1(Keyword, Flav1)           \
  FLAVOR_CHECK(Keyword, FLAVOR_IS(Flav1))

#define FLAVOR_CHECK2(Keyword, Flav1, Flav2)            \
  FLAVOR_CHECK(Keyword, FLAVOR_IS2(Flav1, Flav2))

#define FLAVOR_CHECK3(Keyword, Flav1, Flav2, Flav3)             \
  FLAVOR_CHECK(Keyword, FLAVOR_IS3(Flav1, Flav2, Flav3))

namespace ast
{
  bool
  implicit(const ast::rExp e)
  {
    ast::rConstNoop noop = e.unsafe_cast<const ast::Noop>();
    return noop;
  }

  rExp
  Factory::make_at(const location& loc,
                   const location& flavor_loc, flavor_type flavor,
                   rExp cond,
                   rExp body, rExp onleave,
                   rExp duration) // const
  {
    FLAVOR_CHECK1("at", semicolon);
    if (!onleave)
      onleave = new Noop(loc, 0);

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

  rExp
  Factory::make_event_catcher(const location& loc,
                              EventMatch& event,
                              rExp body, rExp onleave) // const
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
      rExp pattern = new List(loc, event.pattern);
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
                 % bind.result_get().unchecked_cast<Exp>()
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


  rExp
  Factory::make_at_event(const location& loc,
                         const location& flavor_loc, flavor_type flavor,
                         EventMatch& event,
                         rExp body, rExp onleave) // const
  {
    FLAVOR_CHECK1("at", semicolon);
    PARAMETRIC_AST
      (desugar_body,
       "%exp:1 |"
       "waituntil(!'$evt'.active)");
    body = exp(desugar_body % body);
    return make_event_catcher(loc, event, body, onleave);
  }


  rExp
  Factory::make_bin(const location& l,
                    flavor_type op,
                    rExp lhs, rExp rhs) // const
  {
    assert(lhs);
    assert(rhs);
    rExp res = 0;
    switch (op)
    {
    case flavor_pipe:
    {
      rPipe pipe;
      if (pipe = lhs.unsafe_cast<Pipe>())
        pipe->children_get().push_back(rhs);
      else
      {
        pipe = new Pipe(l, exps_type());
        pipe->children_get().push_back(lhs);
        pipe->children_get().push_back(rhs);
      }
      res = pipe;
      break;
    }
    case flavor_and:
    {
      rAnd rand;
      if (rand = lhs.unsafe_cast<And>())
        rand->children_get().push_back(rhs);
      else
      {
        rand = new And(l, exps_type());
        rand->children_get().push_back(lhs);
        rand->children_get().push_back(rhs);
      }
      res = rand;
      break;
    }
    case flavor_comma:
    case flavor_semicolon:
    {
      rNary nary = new Nary(l);
      nary->push_back(lhs, op);
      nary->push_back(rhs);
      res = nary;
      break;
    }
    case flavor_none:
      pabort(op);
    }
    return res;
  }


  rBinding
  Factory::make_binding(const location& l, const
                        bool constant,
                        const location& exp_loc, rExp exp)
  {
    if (false
        // Must be an LValue,
        || !exp.unsafe_cast<LValue>()
        // But must not be one of the subclasses about properties.
        || exp.unsafe_cast<PropertyAction>()
        // Or can be a call without argument, i.e., "Foo.bar".
        || (exp.unsafe_cast<Call>()
            && exp.unsafe_cast<Call>()->arguments_get()))
      SYNTAX_ERROR(exp_loc,
                   "syntax error, %s is not a valid lvalue", *exp);

    rBinding res = new Binding(l, exp.unchecked_cast<LValue>());
    res->constant_set(constant);
    return res;
  }

  rCall
  Factory::make_call(const location& l,
                     libport::Symbol method) // const
  {
    return make_call(l, new Implicit(l), method);
  }


  rCall
  Factory::make_call(const location& l,
                     rExp target,
                     libport::Symbol method, exps_type* args) // const
  {
    return new Call(l, args, target, method);
  }

  /// "<target> . <method> ()".
  rCall
  Factory::make_call(const location& l,
                     rExp target, libport::Symbol method) // const
  {
    return make_call(l, target, method, 0);
  }

  rCall
  Factory::make_call(const location& l,
                     rExp target,
                     libport::Symbol method, rExp arg1) // const
  {
    rCall res = make_call(l, target, method, new exps_type);
    res->arguments_get()->push_back(arg1);
    return res;
  }


  rCall
  Factory::make_call(const location& l,
                     rExp target, libport::Symbol method,
                     rExp arg1, rExp arg2, rExp arg3) // const
  {
    rCall res = make_call(l, target, method, new exps_type);
    res->arguments_get()->push_back(arg1);
    res->arguments_get()->push_back(arg2);
    if (arg3)
      res->arguments_get()->push_back(arg3);
    return res;
  }


  rExp
  Factory::make_closure(rExp value) // const
  {
    PARAMETRIC_AST(a, "closure () { %exp:1 }");
    return exp(a % value);
  }

  rExp
  Factory::make_every(const location&,
                      const location& flavor_loc, flavor_type flavor,
                      rExp test, rExp body) // const
  {
    // every (exp:1) exp:2.
    PARAMETRIC_AST
      (semi,
       "detach ({\n"
       "  var deadline = shiftedTime |\n"
       "  var controlTag = Tag.newFlowControl |\n"
       "  throw\n"
       "  {\n"
       "    controlTag: loop\n"
       "    {\n"
       "      detach ({\n"
       "                try\n"
       "                  { %exp:2 }\n"
       "                catch (var e)\n"
       "                  { controlTag.stop(e)} }) |\n"
       "      deadline += %exp:1 |\n"
       "      sleep (deadline - shiftedTime)\n"
       "    };\n"
       "  }\n"
       "})\n");

    // every| (exp:1) exp:2.
    PARAMETRIC_AST
      (pipe,
       "for (var deadline = shiftedTime; true;\n"
       "     deadline = Control.'every|sleep'(deadline, %exp:1))\n"
       "  %exp:2\n");

    return exp((FLAVOR_IS(semicolon) ? semi
                : FLAVOR_IS(pipe) ? pipe
                : FLAVOR_ERROR("every"))
               % test % body);
  }


  // Build a for(iterable) loop.
  rExp
  Factory::make_for(const location&,
                    const location& flavor_loc, flavor_type flavor,
                    rExp iterable, rExp body) // const
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
    return exp((FLAVOR_IS(and) ? ampersand
                : FLAVOR_IS(pipe) ? pipe
                : FLAVOR_IS(semicolon) ? semi
                : FLAVOR_ERROR("for"))
               % iterable % body);
  }


  // Build a for(var id : iterable) loop.
  rExp
  Factory::make_for(const location& loc,
                    const location& flavor_loc, flavor_type flavor,
                    const location& id_loc, libport::Symbol id,
                    rExp iterable, rExp body) // const
  {
    FLAVOR_CHECK3("for", semicolon, pipe, and);
    return
      new Foreach(loc, flavor,
                       new LocalDeclaration(id_loc, id,
                                                 new Implicit(id_loc)),
                       iterable, make_scope(loc, body));
  }


  // Build a C-like for loop.
  rExp
  Factory::make_for(const location&,
                    const location& flavor_loc, flavor_type flavor,
                    rExp init, rExp test, rExp inc,
                    rExp body) // const
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
    return exp((FLAVOR_IS(semicolon) ? semi
                : FLAVOR_IS(pipe) ? pipe
                : FLAVOR_ERROR("for"))
               % init % inc % test % body);
  }


  rExp
  Factory::make_freezeif(const location&,
                         rExp cond,
                         rExp body) // const
  {
    PARAMETRIC_AST
      (desugar,
       "var '$freezeif_ex' = Tag.new(\"$freezeif_ex\") |"
       "var '$freezeif_in' = Tag.new(\"$freezeif_in\") |"
       "'$freezeif_ex' :"
       "{"
       "  at(%exp:1)"
       "    '$freezeif_in'.freeze"
       "  onleave"
       "    '$freezeif_in'.unfreeze |"
       "  '$freezeif_in' :"
       "  {"
       "    %exp:2 |"
       "    '$freezeif_ex'.stop |"
       "    "
       "  }"
       "}"
        );
    return exp(desugar % cond % body);
  }

  rExp
  Factory::make_if(const location& l,
                   rExp cond,
                   rExp iftrue, rExp iffalse) // const
  {
    return new If(l, make_strip(cond),
                       make_scope(l, iftrue),
                       iffalse ? make_scope(l, iffalse) : new Noop(l, 0));
  }


  // loop %body.
  rExp
  Factory::make_loop(const location& loc,
                     const location& flavor_loc, flavor_type flavor,
                     const location& body_loc, rExp body) // const
  {
    FLAVOR_CHECK2("loop", semicolon, pipe);
    return make_while(loc,
                      flavor_loc, flavor,
                      new Float(loc, 1),
                      body_loc, body);
  }


  rLValue
  Factory::make_lvalue_once(const rLValue& lvalue) // const
  {
    rCall tmp = make_call(lvalue->location_get(), SYMBOL(DOLLAR_tmp));

    if (lvalue->call()->target_implicit())
      return lvalue.get();
    else
      return make_call(lvalue->location_get(), tmp, lvalue->call()->name_get());
  }

  rExp
  Factory::make_lvalue_wrap(const rLValue& lvalue,
                            const rExp& e) // const
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

  rNary
  Factory::make_nary(const location& loc, const rExp& e)
  {
    rNary res = new ast::Nary(loc);
    if (!implicit(e))
      res->push_back(e);
    return res;
  }

  rNary
  Factory::make_nary(const location&,
                     rNary lhs,
                     const location& flavor_loc, flavor_type flavor,
                     const rExp& e)
  {
    if (lhs->back_flavor_get() == ast::flavor_none)
      lhs->back_flavor_set(flavor, flavor_loc);
    if (!implicit(e))
      lhs->push_back(e);
    return lhs;
  }

  rExp
  Factory::make_nil() // const
  {
    PARAMETRIC_AST(nil, "nil");
    return exp(nil);
  }

  namespace
  {
    static local_declarations_type*
    symbols_to_decs(const loc& loc,
                    Factory::formals_type* formals)
    {
      if (!formals)
        return 0;
      local_declarations_type* res = new local_declarations_type();
      foreach (const Factory::formal_type& var, *formals)
        res->push_back(new LocalDeclaration(loc, var.first, var.second));
      delete formals;
      return res;
    }
  }

  rRoutine
  Factory::make_routine(const location& loc, bool closure,
                        const location& floc, formals_type* f,
                        const location& bloc, rExp b) // const
  {
    if (closure && !f)
      SYNTAX_ERROR(loc, "closure cannot be lazy");
    return new Routine(loc, closure,
                            symbols_to_decs(floc, f),
                            make_scope(bloc, b));
  }


  /// Return \a e in a Scope unless it is already one.
  rScope
  Factory::make_scope(const location& l,
                      rExp target, rExp e) // const
  {
    if (rScope res = e.unsafe_cast<Scope>())
      return res;
    else if (target)
      return new Do(l, e, target);
    else
      return new Scope(l, e);
  }

  rScope
  Factory::make_scope(const location& l, rExp e) // const
  {
    return make_scope(l, 0, e);
  }

  rExp
  Factory::make_stopif(const location&,
                       rExp cond, rExp body) // const
  {
    PARAMETRIC_AST
      (desugar,
       "{"
       "  var '$stopif' = Tag.new(\"$stopif\") |"
       "  '$stopif':"
       "  {"
       "    { %exp:2 | '$stopif'.stop } &"
       "    { waituntil(%exp:1) | '$stopif'.stop }"
       "  } |"
       "}"
        );
    return exp(desugar % cond % body);
  }

  rExp
  Factory::make_string(const location& l, libport::Symbol s) // const
  {
    return new String(l, s);
  }


  /*-------------.
  | make_strip.  |
  `-------------*/

  rExp
  Factory::make_strip(rNary nary) // const
  {
    rExp res = nary;
    // Remove useless nary and statement if there's only one child.
    if (nary->children_get().size() == 1)
      res = (nary->children_get().front()
             .unchecked_cast<Stmt>()
             ->expression_get());
    return res;
  }

  rExp
  Factory::make_strip(rExp e) // const
  {
    if (rNary nary = e.unsafe_cast<Nary>())
      return make_strip(nary);
    else
      return e;
  }


  rExp
  Factory::make_switch(const location&, rExp cond,
                       const cases_type& cases, rExp def) // const
  {
    const location& loc = cond->location_get();
    rExp inner = def ? def : make_nil();
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

      rExp condition = c.first->guard_get();
      if (!condition)
        condition = exp(cond);

      rewrite::PatternBinder bind(make_call(loc, SYMBOL(DOLLAR_pattern)), loc);
      bind(c.first->pattern_get().get());

      desugar
        % bind.result_get().unchecked_cast<Exp>()
        % bind.bindings_get()
        % condition
        % bind.bindings_get()
        % c.second
        % inner;
      inner = exp(desugar);
    }

    PARAMETRIC_AST(sw, "{ var '$switch' = %exp:1 | %exp:2 }");
    return exp(sw % cond % inner);
  }

  rExp
  Factory::make_timeout(const rExp& duration,
                        const rExp& body) // const
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


  rExp
  Factory::make_waituntil(const location&,
                          const rExp& cond, rExp duration) // const
  {
    if (duration)
    {
      PARAMETRIC_AST
        (desugar,
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


  rExp
  Factory::make_waituntil_event(const location& loc,
                                rExp event,
                                exps_type* payload) // const
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

    rList d_payload = new List(loc, payload);

    rewrite::PatternBinder bind(make_call(loc, SYMBOL(DOLLAR_pattern)), loc);
    bind(d_payload.get());

    return exp(desugar
               % bind.result_get().unchecked_cast<Exp>()
               % event
               % bind.bindings_get());
  }

  rExp
  Factory::make_whenever_event(const location& loc,
                               EventMatch& event,
                               rExp body, rExp onleave) // const
  {
    PARAMETRIC_AST
      (desugar,
       "while (true)\n"
       "{\n"
       "  %exp:1 |\n"
       "  if(!'$evt'.active)\n"
       "    break\n"
       "}");
    body = exp(desugar % body);
    return make_event_catcher(loc, event, body, onleave);
  }

  rExp
  Factory::make_whenever(const location&,
                         rExp cond,
                         rExp body, rExp else_stmt,
                         rExp duration) // const
  {
    // FIXME: Be smarter on empty else_stmt.
    if (!else_stmt)
      else_stmt = make_nil();
    if (duration)
    {
      PARAMETRIC_AST
        (desugar,
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

  rExp
  Factory::make_while(const location& loc,
                      const location& flavor_loc, flavor_type flavor,
                      rExp cond,
                      const location& body_loc, rExp body) // const
  {
    FLAVOR_CHECK2("while", semicolon, pipe);
    return new While(loc, flavor, cond, make_scope(body_loc, body));
  }

}

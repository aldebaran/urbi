/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
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



#define SYNTAX_ERROR(Loc, ...)                                          \
  throw yy::parser::syntax_error(Loc, libport::format(__VA_ARGS__))

#define FLAVOR_ERROR(Keyword) flavor_error(Keyword, flavor, flavor_loc)

#define FLAVOR_IS(Flav1)                        \
  (flavor == flavor_ ## Flav1)

#define FLAVOR_IS2(Flav1, Flav2)                \
  (FLAVOR_IS(Flav1) || FLAVOR_IS(Flav2))

#define FLAVOR_IS3(Flav1, Flav2, Flav3)                 \
  (FLAVOR_IS2(Flav1, Flav2) || FLAVOR_IS(Flav3))

#define FLAVOR_IS4(Flav1, Flav2, Flav3, Flav4)          \
  (FLAVOR_IS3(Flav1, Flav2, Flav3) || FLAVOR_IS(Flav4))

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

#define FLAVOR_DEFAULT(Flavor)                  \
  do {                                          \
    if (FLAVOR_IS(none))                        \
      flavor = flavor_ ## Flavor;               \
  } while (false)

namespace ast
{
  namespace
  {
    static
    ParametricAst&
    flavor_error(const char* keyword,
                 flavor_type flavor,
                 const Factory::location& flavor_loc)
    {
      SYNTAX_ERROR(flavor_loc, "invalid flavor: %s%s", keyword, flavor);
    }
  }


  bool
  implicit(const rExp e)
  {
    return e.is_a<const Noop>();
  }

  rExp
  Factory::make_assert(const location&,
                       rExp cond) /* const */
  {
    if (rCall call = dynamic_cast<Call*>(cond.get()))
    {
      const location& loc = call->location_get();
      exps_type* args = new exps_type;
      *args << make_string(loc, call->name_get())
            << call->target_get();
      if (exps_type* as = call->arguments_get())
        *args << *as;
      return make_call(loc,
                       make_call(loc, SYMBOL(System)),
                       SYMBOL(assertCall),
                       args);
    }

    PARAMETRIC_AST
      (a,
       "System.'assert'(%exp:1)");
    return exp(a % cond);
  }

  /// assert(%exps).
  rExp
  Factory::make_assert(const location&,
                       exps_type* cond) /* const */
  {
    aver(cond);
    foreach (rExp& c, *cond)
      c = make_assert(c->location_get(), c);
    rNary res = new Nary;
    res->children_set(cond);
    return res;
  }


  rExp
  Factory::make_at(const location& loc,
                   const location& flavor_loc, flavor_type flavor,
                   rExp cond, rExp body, rExp onleave, rExp duration) // const
  {
    FLAVOR_DEFAULT(semicolon);
    FLAVOR_CHECK1("at", semicolon);

    return new At(loc, flavor, flavor_loc,
                  make_strip(cond),
                  make_scope(loc, body),
                  onleave ? make_scope(loc, onleave) : new Noop(loc, 0),
                  duration);
  }

  rExp
  Factory::make_event_catcher(const location& loc,
                              EventMatch& event,
                              rExp enter, rExp leave) // const
  {
    if (!leave)
    {
      PARAMETRIC_AST(noop, "{}");
      leave = exp(noop);
    }

    if (event.duration)
    {
      PARAMETRIC_AST(desugar_event, "%exp:1.persists(%exp:2)");
      event.event = exp(desugar_event % event.event % event.duration);
    }

    rExp guard;
    if (event.guard)
    {
      PARAMETRIC_AST(desugar_guard, "if (%exp:1) '$pattern' else void");
      guard = exp(desugar_guard % event.guard);
    }
    else
    {
      PARAMETRIC_AST(desugar_guard, "'$pattern'");
      guard = exp(desugar_guard);
    }

    if (event.pattern)
    {
      rExp pattern = make_list(loc, event.pattern);
      rewrite::PatternBinder
        bind(make_call(loc, SYMBOL(DOLLAR_pattern)), loc);
      bind(pattern.get());

      rExp positive;
      if (event.guard)
      {
        PARAMETRIC_AST(desugar, "%exp:1| %exp:2");
        positive = exp(desugar % bind.bindings_get() % guard);
      }
      else
      {
        PARAMETRIC_AST(desugar, "'$pattern'");
        positive = exp(desugar);
      }

      PARAMETRIC_AST
        (desugar,
         "{\n"
         "  %exp:1.onEvent(\n"
         "  closure ('$evt')\n"
         "  {\n"
         "    var '$pattern' = Pattern.new(%exp:2) |\n"
         "    if ('$pattern'.match('$evt'.payload))\n"
         "      %exp: 3\n"
         "    else\n"
         "      void\n"
         "  },\n"
         "  closure ('$evt', '$pattern')\n"
         "  {\n"
         "    %exp: 4 |\n"
         "    %exp: 5 |\n"
         "  },\n"
         "  closure ('$evt', '$pattern')\n"
         "  {\n"
         "    %exp: 6 |\n"
         "    %exp: 7 |\n"
         "  })\n"
         "}\n");
      return exp(desugar
                 % event.event
                 % bind.result_get().unchecked_cast<Exp>()
                 % positive
                 % bind.bindings_get()
                 % enter
                 % bind.bindings_get()
                 % leave);
    }
    else
    {
      PARAMETRIC_AST
        (desugar_no_pattern,
         "{\n"
         "  %exp:1.onEvent(\n"
         "  closure ('$evt') { var '$pattern' = true | %exp:2 },\n"
         "  closure ('$evt', '$pattern') { %exp:3 },\n"
         "  closure ('$evt', '$pattern') { %exp:4 })\n"
         "}\n");
      return exp(desugar_no_pattern % event.event % guard % enter % leave);
    }
  }


  rExp
  Factory::make_at_event(const location& loc,
                         const location& flavor_loc, flavor_type flavor,
                         EventMatch& event,
                         rExp body, rExp onleave) // const
  {
    FLAVOR_DEFAULT(semicolon);
    FLAVOR_CHECK1("at", semicolon);
    return make_event_catcher(loc, event, body, onleave);
  }


  rExp
  Factory::make_bin(const location& l,
                    flavor_type op,
                    rExp lhs, rExp rhs) // const
  {
    aver(lhs);
    aver(rhs);
    rExp res = 0;
    switch (op)
    {
    case flavor_pipe:
    {
      rPipe pipe;
      if (pipe = lhs.unsafe_cast<Pipe>())
        pipe->children_get() << rhs;
      else
      {
        pipe = new Pipe(l, exps_type());
        pipe->children_get() << lhs << rhs;
      }
      res = pipe;
      break;
    }
    case flavor_and:
    {
      rAnd rand;
      if (rand = lhs.unsafe_cast<And>())
        rand->children_get() << rhs;
      else
      {
        rand = new And(l, exps_type());
        rand->children_get() << lhs << rhs;
      }
      res = rand;
      break;
    }

    default:
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


  /// "class" lvalue protos block
  rExp
  Factory::make_class(const location& l,
                      rLValue lvalue,
                      exps_type* protos, rExp block) /* const */
  {
    return new Class(l, lvalue, protos, block);
  }

  rExp
  Factory::make_closure(rExp value) // const
  {
    PARAMETRIC_AST(a, "closure () { %exp:1 }");
    return exp(a % value);
  }


  EventMatch
  Factory::make_event_match(const location&,
                            rExp& event,
                            exps_type* args, rExp duration,
                            rExp guard) // const
  {
    return EventMatch(event, args, duration, guard);
  }


  EventMatch
  Factory::make_event_match(const location& l,
                            rExp& event,
                            rExp guard) // const
  {
    exps_type* args = 0;
    rCall call = event.unsafe_cast<Call>();
    if (call && call->arguments_get())
    {
      args = new exps_type(*call->arguments_get());
      call->arguments_set(0);
    }
    return make_event_match(l, event, args, 0, guard);
  }


  rExp
  Factory::make_every(const location&,
                      const location& flavor_loc, flavor_type flavor,
                      rExp test, rExp body) // const
  {
    FLAVOR_DEFAULT(comma);
    // every, (exp:1) exp:2.
    PARAMETRIC_AST
      (comma,
       "for, (var deadline = shiftedTime; true;\n"
       "      deadline = Control.'every,sleep'(deadline, %exp:1))\n"
       "  %exp:2\n");

    // every| (exp:1) exp:2.
    PARAMETRIC_AST
      (pipe,
       "for (var deadline = shiftedTime; true;\n"
       "     deadline = Control.'every|sleep'(deadline, %exp:1))\n"
       "  %exp:2\n");

    return exp((FLAVOR_IS(comma) ? comma
                : FLAVOR_IS(pipe) ? pipe
                : FLAVOR_ERROR("every"))
               % test % body);
  }


  rExp
  Factory::make_external_event_or_function(const location& l,
                                           libport::Symbol kind,
                                           float arity,
                                           libport::Symbol obj,
                                           libport::Symbol slot,
                                           libport::Symbol id) /* const */
  {
    PARAMETRIC_AST
      (event, "'external'.event(%exp:1, %exp:2, %exp:3, %exp:4)");

    PARAMETRIC_AST
      (function, "'external'.'function'(%exp:1, %exp:2, %exp:3, %exp:4)");

    return exp((kind == SYMBOL(event) ? event : function)
               % make_float(l, arity)
               % make_string(l, obj)
               % make_string(l, slot)
               % make_string(l, id));
  }

  rExp
  Factory::make_external_object(const location& l,
                                libport::Symbol id) /* const */
  {
    PARAMETRIC_AST(a, "'external'.object(%exp:1)");
    return exp(a % make_string(l, id));
  }

  rExp
  Factory::make_external_var(const location& l,
                             libport::Symbol obj,
                             libport::Symbol slot,
                             libport::Symbol id) /* const */
  {
    PARAMETRIC_AST(a, "'external'.'var'(%exp:1, %exp:2, %exp:3)");
    return exp(a
               % make_string(l, obj)
               % make_string(l, slot)
               % make_string(l, id));
  }


  rFinally
  Factory::make_finally(const location& l, rExp body, rExp finally) // const
  {
    // This make_scope is mainly for pretty-printing.
    // IMHO there are too many useless rScopes in the AST -- Akim.
    return new Finally(l, make_scope(body), finally);
  }


  rExp
  Factory::make_float(const location& l, libport::ufloat s) // const
  {
    return new Float(l, s);
  }

  // Build a for(iterable) loop.
  rExp
  Factory::make_for(const location&,
                    const location& flavor_loc, flavor_type flavor,
                    rExp iterable, rExp body) // const
  {
    FLAVOR_DEFAULT(semicolon);
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
       "  %exp:2\n"
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
    FLAVOR_DEFAULT(semicolon);
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
    // Include the increment in the condition to be execute it on
    // `continue'.
    //
    // Don't use ";" for costs that should not be visible to the user:
    // $first.
    FLAVOR_DEFAULT(semicolon);
    PARAMETRIC_AST
      (comma,
       "{\n"
       "  %exp:1 |\n"
       "  var '$first' = true |\n"
       "  while, ({ if ('$first') '$first' = false else %exp:2|\n"
       "            %exp:3 })\n"
       "    %exp:4\n"
       "}");

    PARAMETRIC_AST
      (pipe,
       "{\n"
       "  %exp:1 |\n"
       "  var '$first' = true |\n"
       "  while| ({ if ('$first') '$first' = false else %exp:2|\n"
       "            %exp:3 })\n"
       "    %exp:4\n"
       "}");

    PARAMETRIC_AST
      (semicolon,
       "{\n"
       "  %exp:1|\n" // When not entering the loop, we want 0 cycles consumed.
       "  var '$first' = true |\n"
       "  while ({ if ('$first') '$first' = false else %exp:2|\n"
       "           %exp:3 })\n"
       "    %exp:4\n"
       "}");
    return exp((FLAVOR_IS(comma) ? comma
                : FLAVOR_IS(semicolon) ? semicolon
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
       "{"
       "  var '$freezeif_ex' = Tag.new(\"$freezeif_ex\") |"
       "  var '$freezeif_in' = Tag.new(\"$freezeif_in\") |"
       "  '$freezeif_ex' :"
       "  {"
       "    at(%exp:1)"
       "      '$freezeif_in'.freeze"
       "    onleave"
       "      '$freezeif_in'.unfreeze |"
       "    '$freezeif_in' :"
       "    {"
       "      %exp:2 |"
       "      '$freezeif_ex'.stop |"
       "    }"
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


  rList
  Factory::make_list(const location& loc,
                     exps_type* exps) /* const */
  {
    return new List(loc, exps ? exps : new exps_type);
  }

  // loop %body.
  rExp
  Factory::make_loop(const location&,
                     const location& flavor_loc, flavor_type flavor,
                     const location&, rExp body) // const
  {
    FLAVOR_DEFAULT(semicolon);
    PARAMETRIC_AST
      (comma,
       "while, (true)\n"
       "  %exp:1\n"
        );
    PARAMETRIC_AST
      (pipe,
       "while| (true)\n"
       "  %exp:1\n"
        );
    PARAMETRIC_AST
      (semicolon,
       "while; (true)\n"
       "  %exp:1\n"
        );
    return exp((FLAVOR_IS(comma) ? comma
                : FLAVOR_IS(pipe) ? pipe
                : FLAVOR_IS(semicolon) ? semicolon
                : FLAVOR_ERROR("loop"))
               % body);
  }


  rLValue
  Factory::make_lvalue_once(const rLValue& lvalue) // const
  {
    rCall tmp = make_call(lvalue->location_get(), SYMBOL(DOLLAR_tmp));

    if (rCall call = dynamic_cast<Call*>(lvalue.get()))
    {
      if (call->target_implicit())
        return lvalue.get();
      else
        return make_call(lvalue->location_get(), tmp, call->name_get());
    }
    else if (rSubscript sub = dynamic_cast<Subscript*>(lvalue.get()))
    {
      unsigned int i = 0;
      exps_type* args = new exps_type();
      foreach(rExp e, *sub->arguments_get())
        *args << make_call(lvalue->location_get(),
                           libport::Symbol(libport::format("$arg%u", i++)));
      return new Subscript(lvalue->location_get(), args, tmp);
    }

    SYNTAX_ERROR(lvalue->location_get(),
                 "syntax error, %s is not a valid lvalue", lvalue);
    return 0;
  }

  rExp
  Factory::make_lvalue_wrap(const rLValue& lvalue,
                            const rExp& e) // const
  {
    PARAMETRIC_AST
      (wrap,
       "{\n"
       "  var '$tmp' = %exp:1 |\n"
       "  %exp:2;\n"
       "}\n");

    if (rCall call = dynamic_cast<Call*>(lvalue.get()))
    {
      if (call->target_implicit())
        return e;
      else
      {
        wrap % call->target_get() % e;
        return exp(wrap);
      }
    }
    else if (rSubscript sub = dynamic_cast<Subscript*>(lvalue.get()))
    {
      rExp result = e;

      unsigned int i = 0;
      exps_type* args = sub->arguments_get();
      foreach(rExp e, *args)
      {
        PARAMETRIC_AST
          (arg,
           "var %id:1 = %exp:2;\n"
           "%exp:3");
        arg % libport::Symbol(libport::format("$arg%u", i++))
          % e
          % result;
        result = exp(arg);
      }

      wrap % sub->target_get() % result;
      return exp(wrap);
    }

    SYNTAX_ERROR(lvalue->location_get(),
                 "syntax error, %s is not a valid lvalue", lvalue);
    return 0;
  }

  rNary
  Factory::make_nary(const location& loc, const rExp& e)
  {
    rNary res = new Nary(loc);
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
    if (lhs->back_flavor_get() == flavor_none)
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

  /// Create a Position.
  rExp
  Factory::make_position(const location& loc) /* const */
  {
    PARAMETRIC_AST(pos, "Position.new(%exp:1, %exp:2, %exp:3)");
    const libport::Symbol* fn = loc.begin.filename;
    return exp(pos
               % (fn ? make_string(loc, fn->name_get()) : make_nil())
               % make_float(loc, loc.begin.line)
               % make_float(loc, loc.begin.column));
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
    // If we have a Scope and a target, maybe we should convert this
    // scope to a Do node? But are we sure we won't loose any property
    // from the original Scope node? For now, put the scope in a Do.
    rScope res;
    if ((res = e.unsafe_cast<Scope>()) && !target)
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

  rScope
  Factory::make_scope(rExp e) // const
  {
    if (e)
      return make_scope(e->location_get(), e);
    else
      return 0;
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
  Factory::make_string(const location& l, const std::string& s) // const
  {
    return new String(l, s);
  }


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

  rThrow
  Factory::make_throw(const location& l, const rExp& e) /* const */
  {
    return new Throw(l, e);
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

  rTry
  Factory::make_try(const location& loc,
                    rExp body,
                    const catches_type& catches, rExp elseclause) /* const */
  {
    return new Try(loc,
                   make_scope(body), catches,
                   make_scope(elseclause));
  }

  rExp
  Factory::make_try(const location& loc,
                    rExp body,
                    const catches_type& catches, rExp elseclause,
                    rExp finally) /* const */
  {
    rExp res = make_try(loc, body, catches, elseclause);
    if (finally)
      res = make_finally(loc, res, finally);
    return res;
  }

  // (a, b, c) --> Tuple.new([a, b, c])
  rExp
  Factory::make_tuple(const location& loc,
                      exps_type* exps) // const
  {
    PARAMETRIC_AST(ast, "Tuple.new(%exp:1)");
    return exp(ast % make_list(loc, exps));
  }

  rExp
  Factory::make_waituntil(const location&, const rExp& cond, rExp duration)
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
      PARAMETRIC_AST(stop_ast, "'$waituntil'.stop");
      static const rExp stop = exp(stop_ast);

      PARAMETRIC_AST
        (desugar,
         "{"
         "  var '$waituntil' = Tag.new |"
         "  '$waituntil': { at (%exp:1) %exp:2 | sleep(inf) } |"
         "}");
      ;
      return exp(desugar % cond % stop);
    }
  }


  rExp
  Factory::make_waituntil_event(const location& loc,
                                EventMatch& event) // const
  {
    if (!event.pattern)
    {
      PARAMETRIC_AST(desugar, "%exp:1.'waituntil'(nil)");
      return exp(desugar % event.event);
    }

    rList d_payload = make_list(loc, event.pattern);

    rewrite::PatternBinder bind(make_call(loc, SYMBOL(DOLLAR_pattern)), loc);
    bind(d_payload.get());

    rExp guard;
    if (event.guard)
    {
      PARAMETRIC_AST(desugar_guard, "closure (var '$pattern') { %exp:1 | %exp:2 }");
      guard = exp(desugar_guard % bind.bindings_get() % event.guard);
    }
    else
    {
      PARAMETRIC_AST(desugar_guard, "nil");
      guard = exp(desugar_guard);
    }

    PARAMETRIC_AST
      (desugar,
       "{\n"
       "  var '$pattern' = Pattern.new(%exp:1, %exp:4) |\n"
       "  %exp:2.'waituntil'('$pattern') |\n"
       "  {\n"
       "    %unscope: 2 |\n"
       "    %exp:3 |\n"
       "  }\n"
       "}");

    return exp(desugar
               % bind.result_get().unchecked_cast<Exp>()
               % event.event
               % bind.bindings_get()
               % guard);
  }

  rExp
  Factory::make_whenever_event(const location& loc,
                               EventMatch& event,
                               rExp body, rExp _else) // const
  {
    if (_else)
    {
      PARAMETRIC_AST
        (desugar_body_ast,
         "'$mtx': if ('$else')"
         "{"
         "  '$else' = false |"
         "  '!$mtx': waituntil('$switch'?)"
         "}|"
         "'$count'++|"
         "while (true)"
         "{"
         "  %exp:1;"
         "  if (!'$evt'.active)"
         "    break;"
         "}|");
      rExp desugar_body = exp(desugar_body_ast % body);

      PARAMETRIC_AST
        (desugar_leave_ast,
         "'$mtx':"
         "{"
         "  '$count'--|"
         "  if (!'$count')"
         "  {"
         "    '$else' = true|"
         "    '$switch'!|"
         "  }"
         "}");
      rExp desugar_leave = exp(desugar_leave_ast);

      rExp at = make_event_catcher(loc, event, desugar_body, desugar_leave);

      PARAMETRIC_AST
        (desugar,
         "{"
         "  var '$else' = true|"
         "  var '$switch' = Event.new|"
         "  var '$mtx' = Mutex.new|"
         "  var '!$mtx' = !'$mtx'|"
         "  var '$count' = 0|"
         "  %exp:1|" // at
         "  while (true)"
         "  {"
         "    '$mtx': if (!'$else')"
         "    {"
         "      '$switch'!;"
         "      '!$mtx': waituntil('$switch'?)|"
         "    }|"
         "    %exp:2|"
         "  }"
         "}");

      return exp(desugar % at % _else);
    }
    else
    {
      PARAMETRIC_AST
        (desugar_body_ast,
         "while (true)"
         "{"
         "  %exp:1;"
         "  if (!'$evt'.active)"
         "    break;"
         "}|");
      rExp desugar_body = exp(desugar_body_ast % body);

      return make_event_catcher(loc, event, desugar_body, 0);
    }
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
      PARAMETRIC_AST
        (desugar,
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
    FLAVOR_DEFAULT(semicolon);
    FLAVOR_CHECK3("while", comma, pipe, semicolon);
    return new While(loc, flavor, cond, make_scope(body_loc, body));
  }

}

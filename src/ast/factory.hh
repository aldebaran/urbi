/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef AST_FACTORY_HH
# define AST_FACTORY_HH

# include <list>
# include <libport/fwd.hh>
# include <libport/ufloat.hh>

# include <ast/catches-type.hh>
# include <ast/exps-type.hh>
# include <ast/flavor.hh>
# include <ast/formal.hh>
# include <ast/fwd.hh>
# include <ast/symbols-type.hh>

namespace ast
{

  class Factory
  {
  public:
    typedef std::pair<libport::Symbol, rExp> modifier_type;

    typedef yy::location location;

    /// assert(%exp).
    static
    rExp
    make_assert(const location& loc,
                rExp cond) /* const */;

    /// assert(%exps).
    /// Note that \a cond is "reclaimed" do not use it again.
    /// (Actually it is reused to build the result).
    static
    rExp
    make_assert(const location& loc,
                exps_type* cond) /* const */;

    /// at%flavor (%cond ~ %duration) {%body} onleave {%onleave}
    static
    rExp
    make_at(const location& loc,
            const location& flavor_loc,
            flavor_type flavor,
            const ast::symbols_type& args,
            rExp cond,
            rExp body, rExp onleave = 0,
            rExp duration = 0) /* const */;

    /// at(%flavor) (%args) (?(%event)(%payload) {%body} onleave {%onleave}
    static
    rExp
    make_at_event(const location& loc,
                  const location& flavor_loc,
                  flavor_type flavor,
                  const ast::symbols_type& args,
                  EventMatch& event,
                  rExp body, rExp onleave = 0) /* const */;
    static
    rExp
    make_at_event(const location& loc,
                  const location& flavor_loc,
                  flavor_type flavor,
                  bool sync,
                  EventMatch& event,
                  rExp body, rExp onleave = 0) /* const */;

    static
    rExp
    make_watch(const location& loc, ast::rExp exp);

    /// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
    /// \param op can be flavor_and or flavor_pipe.
    static
    rExp
    make_bin(const location& l,
             flavor_type op,
             rExp lhs, rExp rhs) /* const */;

    /// Create a binding, possibly const.
    /// Corresponds to "var <exp>" and "const var <exp>".
    /// Check that <exp> can indeed be declared: it must be an LValue.
    /// It must not be a descendant such as Property.
    static
    rBinding
    make_binding(const location& l,
                 bool constant,
                 const location& exp_loc, rExp exp) /* const */;

    /// Translate &&.
    static
    rExp
    make_and(const location& l, rExp lhs, rExp rhs) /* const */;

    /// "<method>"
    static
    rCall
    make_call(const location& l,
              libport::Symbol method) /* const */;


    /// "<target> . <method> (args)".
    static
    rCall
    make_call(const location& l,
              rExp target,
              libport::Symbol method, exps_type* args) /* const */;

    /// "<target> . <method> ()".
    static
    rCall
    make_call(const location& l,
              rExp target, libport::Symbol method) /* const */;

    /// "<target> . <method> (<arg1>)".
    static
    rCall
    make_call(const location& l,
              rExp target, libport::Symbol method, rExp arg1) /* const */;


    /// "<target> . <method> (<arg1>, <arg2>)".
    /// "<target> . <method> (<arg1>, <arg2>, <arg3>)".
    static
    rCall
    make_call(const location& l,
              rExp target, libport::Symbol method,
              rExp arg1, rExp arg2, rExp arg3 = 0) /* const */;

    /// "<detach-or-disown> (<body>)".
    static
    rExp
    make_detach(const location& l,
                bool is_detach, rExp body) /* const */;

    /// "<target> :: <member>".
    static
    rExp
    make_get_slot(const location& l,
                  rExp target, libport::Symbol member);

    /// ":: <member>".
    static
    rExp
    make_get_slot(const location& l,
                  libport::Symbol member);

    /// "class" lvalue protos block
    static
    rExp
    make_class(const location& l,
               rLValue lvalue,
               exps_type* protos, rExp block) /* const */;


    /// closure () { <value> }
    static
    rExp
    make_closure(rExp value) /* const */;

    /// "enum" "{" ids "}"
    static
    rExp
    make_enum(const yy::location& l,
              libport::Symbol name,
              const symbols_type& ids) /* const */;

    static
    rExp
    make_event_catcher(const location& loc,
                       EventMatch& event,
                       rExp body, rExp onleave,
                       bool sync = false) /* const */;

    /// <event> "?" <args> ~ <duration> if <guard>.
    static
    EventMatch
    make_event_match(const location&,
                     rExp& event, exps_type* args,
                     rExp duration, rExp guard);

    /// Backward compatibility.
    /// "?" <exp> <guard>
    static
    EventMatch
    make_event_match(const location&,
                     rExp& event, rExp guard); // const

    /// Build an "every" loop.
    static
    rExp
    make_every(const location& loc,
               const location& flavor_loc, flavor_type flavor,
               rExp test, rExp body) /* const */;


    /// external event, or external function.
    static
    rExp
    make_external_event_or_function(const location& loc,
                                    libport::Symbol kind,
                                    float arity,
                                    libport::Symbol obj,
                                    libport::Symbol slot,
                                    libport::Symbol id) /* const */;

    static
    rExp
    make_external_object(const location& l,
                         libport::Symbol id) /* const */;

    static
    rExp
    make_external_var(const location& l,
                      libport::Symbol obj,
                      libport::Symbol slot,
                      libport::Symbol id) /* const */;

    static
    rFinally
    make_finally(const location& l, rExp body, rExp finally) /* const */;

    static
    rExp
    make_float(const location& l, libport::ufloat s) /* const */;


    /// Build a for(iterable) loop.
    static
    rExp
    make_for(const location&,
             const location& flavor_loc, flavor_type flavor,
             rExp iterable, rExp body) /* const */;

    // Build a for(var id : iterable) loop.
    static
    rExp
    make_for(const location& l,
             const location& flavor_loc, flavor_type flavor,
             const location& id_loc, libport::Symbol id,
             rExp iterable, rExp body) /* const */;

    /// Build a C-for loop.
    static
    rExp
    make_for(const location& l,
             const location& flavor_loc, flavor_type flavor,
             rExp init, rExp test, rExp inc,
             rExp body) /* const */;


    /// freezeif ( %cond ) %body.
    static
    rExp
    make_freezeif(const location& loc,
                  rExp cond, rExp body) /* const */;

    /// \param iffalse can be 0.
    static
    rExp
    make_if(const location& l,
            rExp cond, rExp iftrue, rExp iffalse) /* const */;

    /// isdef(call)
    static
    rExp
    make_isdef(const location& l, rCall call) /* const */;


    /// Make a list.
    /// \param loc    the location of the whole list, "[", "]" included.
    /// \param exps   the list of expressions.  Stolen.
    static
    rList
    make_list(const location& loc,
              exps_type* exps = 0) /* const */;

    /// loop %body.
    static
    rExp
    make_loop(const location& loc,
              const location& flavor_loc, flavor_type flavor,
              rExp body) /* const */;

    /** Use these functions to avoid CPP-like problem when referring
     *  several times to an lvalue.  For instance, do not desugar
     *
     *  f(x).val += 1
     *
     *  as
     *
     *  f(x).val = f(x).val + 1
     *
     *  but as
     *
     *  var tmp = f(x) | tmp.val = tmp.val + 1
     static
     *
     * 1/ Use make_lvalue_once to get the actual slot owner to use.
     * 2/ Transform your code
     * 3/ Use make_lvalue_wrap to potentially define the cached owner.
     *
     */
    // We need two separate functions. This could be improved if
    // ParametricAst where able to pick the same variable several times,
    // which should pose no problem now that ast are refcounted.
    static
    rLValue
    make_lvalue_once(const rLValue& lvalue) /* const */;
    static
    rExp
    make_lvalue_wrap(const rLValue& lvalue, const rExp& e) /* const */;

    static
    rNary
    make_nary(const location& loc, const rExp& e,
	      flavor_type flavor = flavor_none);

    /// Append \a e to \a lhs using \a flavor, and return \a lhs.
    /// \return \a lhs, modified.
    static
    rNary
    make_nary(const location& loc,
              rNary lhs,
              const location& flavor_loc, flavor_type flavor,
              const rExp& e);

    static
    /// Return the ast for "nil".
    rExp make_nil() /* const */;

    /// Return "do nothing".
    static
    rNoop make_noop(const location& l);

    /// Translate ||.
    static
    rExp
    make_or(const location& l, rExp lhs, rExp rhs) /* const */;

    /// Create a Position.
    static
    rExp make_position(const location& loc) /* const */;

    /// Create a closure or a function.
    /// \param closure  whether building a closure.
    /// \param loc      location for the whole declaration.
    /// \param floc     location of the formals.
    /// \param f        formals.  Mandatory for closures.
    /// \param b        body.
    static
    rRoutine
    make_routine(const location& loc, bool closure,
                 const location& floc, Formals* f,
                 const rExp b) /* const */;

    /// Return \a e in a Scope unless it is already one.
    static
    rScope
    make_scope(const location& l, rExp target, rExp e) /* const */;

    static
    rScope
    make_scope(const location& l, rExp e) /* const */;

    // Use the location of \a e.
    static
    rScope
    make_scope(rExp e) /* const */;

    static
    rExp
    make_stopif(const location& loc,
                rExp cond, rExp body) /* const */;

    static
    rExp
    make_string(const location& l, const std::string& s) /* const */;


    /* Simplify \a e in every possible means.  Typically, remove useless
       static
       Naries with a single statement.  */
    static
    rExp make_strip(rNary e) /* const */;
    static
    rExp make_strip(rExp e) /* const */;


    /*--------.
    | Switch  |
    `--------*/

    typedef std::pair<rMatch, rNary> case_type;
    typedef std::list<case_type> cases_type;

    static
    rExp
    make_switch(const yy::location& l, rExp cond,
                const cases_type& cases, rExp def) /* const */;


    /// throw e;
    static
    rThrow
    make_throw(const location& l, const rExp& e) /* const */;

    /// timeout(duration) body
    static
    rExp
    make_timeout(const rExp& duration, const rExp& body) /* const */;

    // try <stmt> <catch>+ <else>?
    static
    rTry
    make_try(const location& loc,
             rExp body,
             const catches_type& catches = catches_type(),
             rExp elseclause = 0) /* const */;

    // try <stmt> <catch>+ <else>? <finally>?
    static
    rExp
    make_try(const location& loc,
             rExp body,
             const catches_type& catches, rExp elseclause,
             rExp finally) /* const */;

    // (a, b, c) --> Tuple.new([a, b, c])
    static
    rExp
    make_tuple(const location& loc,
              exps_type* exps = 0) /* const */;


    /// waituntil (cond ~ duration);
    /// \param duration can be 0.
    static
    rExp
    make_waituntil(const location& loc,
                   const rExp& cond, rExp duration) /* const */;

    /// waituntil (?(%event)(%payload))
    static
    rExp
    make_waituntil_event(const location& loc,
                         EventMatch& event) /* const */;

    /// whenever (%cond ~ %duration) {%body} onleave {%else_stmt}
    static
    rExp
    make_whenever(const location& loc,
                  rExp cond,
                  rExp body, rExp else_stmt = 0,
                  rExp duration = 0) /* const */;

    /// whenever (?(%event)(%payload) {%body} onleave {%onleave}
    static
    rExp
    make_whenever_event(const location& loc,
                        EventMatch& event,
                        rExp body, rExp onleave = 0) /* const */;

    /// while (%cond) %body.
    static
    rExp
    make_while(const location& loc,
               const location& flavor_loc, flavor_type flavor,
               rExp cond,
               rExp body) /* const */;
  };

  /// Whether the \a e was the empty command.
  bool implicit(const rExp e);

}

// The structures (list and pair) live in std, that's where Koening
// look-up will look for them.
namespace std
{
  ostream& operator<<(ostream& o, const ast::Factory::case_type& c);
  ostream& operator<<(ostream& o, const ast::Factory::cases_type& c);
  ostream& operator<<(ostream& o, const ast::Factory::modifier_type& m);
}

#endif // !AST_FACTORY_HH

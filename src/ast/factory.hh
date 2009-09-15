/*
 * Copyright (C) 2009, Gostai S.A.S.
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

# include <ast/exps-type.hh>
# include <ast/flavor.hh>
# include <ast/fwd.hh>

namespace ast
{

  class Factory
  {
  public:
    typedef std::pair<libport::Symbol, rExp> modifier_type;
    typedef modifier_type formal_type;
    typedef std::vector<formal_type> formals_type;

    typedef yy::location location;

    /// at%flavor (%cond ~ %duration) {%body} onleave {%onleave}
    static
    rExp
    make_at(const yy::location& loc,
            const location& flavor_loc, flavor_type flavor,
            rExp cond,
            rExp body, rExp onleave = 0,
            rExp duration = 0) /* const */;

    /// at(%flavor) (?(%event)(%payload) {%body} onleave {%onleave}
    static
    rExp
    make_at_event(const location& loc,
                  const location& flavor_loc, flavor_type flavor,
                  EventMatch& event,
                  rExp body, rExp onleave = 0) /* const */;

    /// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
    /// \param op can be any of the four cases.
    static
    rExp
    make_bin(const yy::location& l,
             flavor_type op,
             rExp lhs, rExp rhs) /* const */;

    /// Create a binding, possibly const.
    /// Corresponds to "var <exp>" and "const var <exp>".
    /// Check that <exp> can indeed be declared: it must be an LValue.
    /// It must not be a descendant such as Property.
    static
    rBinding
    make_binding(const yy::location& l,
                 bool constant,
                 const yy::location& exp_loc, rExp exp) /* const */;

    /// "<method>"
    static
    rCall
    make_call(const yy::location& l,
              libport::Symbol method) /* const */;


    /// "<target> . <method> (args)".
    static
    rCall
    make_call(const yy::location& l,
              rExp target,
              libport::Symbol method, exps_type* args) /* const */;

    /// "<target> . <method> ()".
    static
    rCall
    make_call(const yy::location& l,
              rExp target, libport::Symbol method) /* const */;

    /// "<target> . <method> (<arg1>)".
    static
    rCall
    make_call(const yy::location& l,
              rExp target, libport::Symbol method, rExp arg1) /* const */;


    /// "<target> . <method> (<arg1>, <arg2>)".
    /// "<target> . <method> (<arg1>, <arg2>, <arg3>)".
    static
    rCall
    make_call(const yy::location& l,
              rExp target, libport::Symbol method,
              rExp arg1, rExp arg2, rExp arg3 = 0) /* const */;


    /// "class" lvalue protos block
    static
    rExp
    make_class(const yy::location& l,
               rCall lvalue,
               exps_type* protos, rExp block) /* const */;


    /// closure () { <value> }
    static
    rExp
    make_closure(rExp value) /* const */;

    static
    rExp
    make_event_catcher(const location& loc,
                       EventMatch& event,
                       rExp body, rExp onleave) /* const */;

    /// Build an "every" loop.
    static
    rExp
    make_every(const yy::location& loc,
               const location& flavor_loc, flavor_type flavor,
               rExp test, rExp body) /* const */;

    // Build a for(iterable) loop.
    static
    rExp
    make_for(const yy::location&,
             const location& flavor_loc, flavor_type flavor,
             rExp iterable, rExp body) /* const */;

    // Build a for(var id : iterable) loop.
    static
    rExp
    make_for(const yy::location& l,
             const location& flavor_loc, flavor_type flavor,
             const yy::location& id_loc, libport::Symbol id,
             rExp iterable, rExp body) /* const */;

    /// Build a C-for loop.
    static
    rExp
    make_for(const yy::location& l,
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
    make_if(const yy::location& l,
            rExp cond, rExp iftrue, rExp iffalse) /* const */;

    /// loop %body.
    static
    rExp
    make_loop(const location& loc,
              const location& flavor_loc, flavor_type flavor,
              const location& body_loc, rExp body) /* const */;

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
    make_nary(const location& loc, const rExp& e);

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

    /// Create a closure or a function.
    /// \param closure  whether building a closure.
    /// \param loc      location for the whole declaration.
    /// \param floc     location of the formals.
    /// \param f        formals.  Mandatory for closures.
    /// \param bloc     location of the body.
    /// \param b        body.
    static
    rRoutine
    make_routine(const location& loc, bool closure,
                 const location& floc, formals_type* f,
                 const location& bloc, const rExp b) /* const */;

    /// Return \a e in a Scope unless it is already one.
    static
    rScope
    make_scope(const yy::location& l, rExp target, rExp e) /* const */;

    static
    rScope
    make_scope(const yy::location& l, rExp e) /* const */;

    static
    rExp
    make_stopif(const location& loc,
                rExp cond, rExp body) /* const */;

    static
    rExp
    make_string(const yy::location& l, libport::Symbol s) /* const */;


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


    /// timeout(duration) body
    static
    rExp
    make_timeout(const rExp& duration, const rExp& body) /* const */;

    /// waituntil (cond ~ duration);
    /// \param duration can be 0.
    static
    rExp
    make_waituntil(const yy::location& loc,
                   const rExp& cond, rExp duration) /* const */;

    /// waituntil (?(%event)(%payload))
    static
    rExp
    make_waituntil_event(const location& loc,
                         rExp event,
                         exps_type* payload) /* const */;

    /// whenever (%cond ~ %duration) {%body} onleave {%else_stmt}
    static
    rExp
    make_whenever(const yy::location& loc,
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
               const location& body_loc, rExp body) /* const */;
  };

  /// Whether the \a e was the empty command.
  bool implicit(const ast::rExp e);

}

// The structures (list and pair) live in std, that's where Koening
// look-up will look for them.
namespace std
{
  ostream& operator<<(ostream& o, const ast::Factory::case_type& c);
  ostream& operator<<(ostream& o, const ast::Factory::cases_type& c);
  ostream& operator<<(ostream& o, const ast::Factory::modifier_type& m);
  ostream& operator<<(ostream& o, const ast::Factory::formals_type& f);
}

#endif // !AST_FACTORY_HH

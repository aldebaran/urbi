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
    typedef std::pair<libport::Symbol, ast::rExp> modifier_type;
    typedef modifier_type formal_type;
    typedef std::vector<formal_type> formals_type;

    typedef yy::location location;

    /// at%flavor (%cond ~ %duration) {%body} onleave {%onleave}
    static
    ast::rExp
    make_at(const yy::location& loc,
            const location& flavor_loc, ast::flavor_type flavor,
            ast::rExp cond,
            ast::rExp body, ast::rExp onleave = 0,
            ast::rExp duration = 0) /* const */;

    /// at(%flavor) (?(%event)(%payload) {%body} onleave {%onleave}
    static
    ast::rExp
    make_at_event(const location& loc,
                  const location& flavor_loc, ast::flavor_type flavor,
                  EventMatch& event,
                  ast::rExp body, ast::rExp onleave = 0) /* const */;

    /// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
    /// \param op can be any of the four cases.
    static
    ast::rExp
    make_bin(const yy::location& l,
             ast::flavor_type op,
             ast::rExp lhs, ast::rExp rhs) /* const */;

    /// Create a binding, possibly const.
    /// Corresponds to "var <exp>" and "const var <exp>".
    /// Check that <exp> can indeed be declared: it must be an LValue.
    /// It must not be a descendant such as Property.
    static
    ast::rBinding
    make_binding(const yy::location& l,
                 bool constant,
                 const yy::location& exp_loc, ast::rExp exp) /* const */;

    /// "<method>"
    static
    ast::rCall
    make_call(const yy::location& l,
              libport::Symbol method) /* const */;


    /// "<target> . <method> (args)".
    static
    ast::rCall
    make_call(const yy::location& l,
              ast::rExp target,
              libport::Symbol method, ast::exps_type* args) /* const */;

    /// "<target> . <method> ()".
    static
    ast::rCall
    make_call(const yy::location& l,
              ast::rExp target, libport::Symbol method) /* const */;

    /// "<target> . <method> (<arg1>)".
    static
    ast::rCall
    make_call(const yy::location& l,
              ast::rExp target, libport::Symbol method, ast::rExp arg1) /* const */;


    /// "<target> . <method> (<arg1>, <arg2>)".
    /// "<target> . <method> (<arg1>, <arg2>, <arg3>)".
    static
    ast::rCall
    make_call(const yy::location& l,
              ast::rExp target, libport::Symbol method,
              ast::rExp arg1, ast::rExp arg2, ast::rExp arg3 = 0) /* const */;


    /// "class" lvalue protos block
    static
    ast::rExp
    make_class(const yy::location& l,
               ast::rCall lvalue,
               ast::exps_type* protos, ast::rExp block) /* const */;


    /// closure () { <value> }
    static
    ast::rExp
    make_closure(ast::rExp value) /* const */;

    static
    ast::rExp
    make_event_catcher(const location& loc,
                       EventMatch& event,
                       ast::rExp body, ast::rExp onleave) /* const */;

    /// Build an "every" loop.
    static
    ast::rExp
    make_every(const yy::location& loc,
               const location& flavor_loc, ast::flavor_type flavor,
               ast::rExp test, ast::rExp body) /* const */;

    // Build a for(iterable) loop.
    static
    ast::rExp
    make_for(const yy::location&,
             const location& flavor_loc, ast::flavor_type flavor,
             ast::rExp iterable, ast::rExp body) /* const */;

    // Build a for(var id : iterable) loop.
    static
    ast::rExp
    make_for(const yy::location& l,
             const location& flavor_loc, ast::flavor_type flavor,
             const yy::location& id_loc, libport::Symbol id,
             ast::rExp iterable, ast::rExp body) /* const */;

    /// Build a C-for loop.
    static
    ast::rExp
    make_for(const yy::location& l,
             const location& flavor_loc, ast::flavor_type flavor,
             ast::rExp init, ast::rExp test, ast::rExp inc,
             ast::rExp body) /* const */;


    /// freezeif ( %cond ) %body.
    static
    ast::rExp
    make_freezeif(const location& loc,
                  ast::rExp cond, ast::rExp body) /* const */;

    /// \param iffalse can be 0.
    static
    ast::rExp
    make_if(const yy::location& l,
            ast::rExp cond, ast::rExp iftrue, ast::rExp iffalse) /* const */;

    /// loop %body.
    static
    ast::rExp
    make_loop(const location& loc,
              const location& flavor_loc, ast::flavor_type flavor,
              const location& body_loc, ast::rExp body) /* const */;

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
    ast::rLValue
    make_lvalue_once(const ast::rLValue& lvalue) /* const */;
    static
    ast::rExp
    make_lvalue_wrap(const ast::rLValue& lvalue, const ast::rExp& e) /* const */;

    static
    /// Return the ast for "nil".
    ast::rExp make_nil() /* const */;

    /// Create a closure or a function.
    /// \param closure  whether building a closure.
    /// \param loc      location for the whole declaration.
    /// \param floc     location of the formals.
    /// \param f        formals.  Mandatory for closures.
    /// \param bloc     location of the body.
    /// \param b        body.
    static
    ast::rRoutine
    make_routine(const location& loc, bool closure,
                 const location& floc, formals_type* f,
                 const location& bloc, const ast::rExp b) /* const */;

    /// Return \a e in a ast::Scope unless it is already one.
    static
    ast::rScope
    make_scope(const yy::location& l, ast::rExp target, ast::rExp e) /* const */;

    static
    ast::rScope
    make_scope(const yy::location& l, ast::rExp e) /* const */;

    static
    ast::rExp
    make_stopif(const location& loc,
                ast::rExp cond, ast::rExp body) /* const */;

    static
    ast::rExp
    make_string(const yy::location& l, libport::Symbol s) /* const */;


    /* Simplify \a e in every possible means.  Typically, remove useless
       static
       Naries with a single statement.  */
    static
    ast::rExp make_strip(ast::rNary e) /* const */;
    static
    ast::rExp make_strip(ast::rExp e) /* const */;


    /*--------.
    | Switch  |
    `--------*/

    typedef std::pair<ast::rMatch, ast::rNary> case_type;
    typedef std::list<case_type> cases_type;

    static
    ast::rExp
    make_switch(const yy::location& l, ast::rExp cond,
                const cases_type& cases, ast::rExp def) /* const */;


    /// timeout(duration) body
    static
    ast::rExp
    make_timeout(const ast::rExp& duration, const ast::rExp& body) /* const */;

    /// waituntil (cond ~ duration);
    /// \param duration can be 0.
    static
    ast::rExp
    make_waituntil(const yy::location& loc,
                   const ast::rExp& cond, ast::rExp duration) /* const */;

    /// waituntil (?(%event)(%payload))
    static
    ast::rExp
    make_waituntil_event(const location& loc,
                         ast::rExp event,
                         ast::exps_type* payload) /* const */;

    /// whenever (%cond ~ %duration) {%body} onleave {%else_stmt}
    static
    ast::rExp
    make_whenever(const yy::location& loc,
                  ast::rExp cond,
                  ast::rExp body, ast::rExp else_stmt = 0,
                  ast::rExp duration = 0) /* const */;

    /// whenever (?(%event)(%payload) {%body} onleave {%onleave}
    static
    ast::rExp
    make_whenever_event(const location& loc,
                        EventMatch& event,
                        ast::rExp body, ast::rExp onleave = 0) /* const */;

    /// while (%cond) %body.
    static
    ast::rExp
    make_while(const location& loc,
               const location& flavor_loc, ast::flavor_type flavor,
               ast::rExp cond,
               const location& body_loc, ast::rExp body) /* const */;
  };

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

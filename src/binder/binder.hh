/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file binder/binder.hh
 ** \brief Definition of binder::Binder.
 */

#ifndef BINDER_BINDER_HH
# define BINDER_BINDER_HH

# include <libport/finally.hh>
# include <list>
# include <boost/unordered_map.hpp>

# include <ast/all.hh>
# include <ast/analyzer.hh>
# include <binder/bind.hh>
# include <urbi/object/fwd.hh>

namespace binder
{

  /// Ast local variables binder.
  class Binder : public ast::Analyzer
  {
  public:
    /// \name Useful shorthands.
    /// \{
    /// Super class type.
    typedef ast::Analyzer super_type;
    /// Import rObject
    typedef urbi::object::rObject rObject;
    /// \}

    /// \name Ctor & dtor.
    /// \{
    /// Construct a \c Binder.
    Binder();

    /// Destroy a Binder.
    virtual ~Binder();
    /// \}

    /// Import visit from DefaultVisitor.
    using super_type::visit;

  protected:
    CONST_VISITOR_VISIT_NODES(
                              (Assignment)
                              (Call)
                              (CallMsg)
                              (Catch)
                              (Declaration)
                              (Dictionary)
                              (Do)
                              (Foreach)
                              (If)
                              (LocalDeclaration)
                              (Match)
                              (Routine)
                              (Scope)
                              (Unscope)
                              (While)
      );

    template <typename Node, typename NewNode>
    void link_to_declaration(Node input,
                             NewNode current,
                             libport::Symbol name,
                             unsigned depth);

  private:
    /// Report an error.
    void err(const ast::loc& loc, const std::string& msg);
    /// Actions to perform at exit of the innermost scope.
    typedef std::list<libport::Finally> unbind_type;
    unbind_type unbind_;

    /// Declaration * (routine_depth * scope_depth)
    typedef std::pair<ast::rLocalDeclaration,
                      std::pair<unsigned, unsigned> > binding_type;
    typedef std::list<binding_type> Bindings;
    typedef boost::unordered_map<libport::Symbol, Bindings> Environment;
    /// Map of currently bound variables
    Environment env_;

    /// Whether to apply setSlot on self
    typedef std::vector<bool> set_on_self_type;
    set_on_self_type setOnSelf_;
    bool set_on_self(unsigned up = 0);

    /// \name Routine stack.
    /// \{
    /// The stack of current number of local variables, and maximum
    /// number of local variable used by the current routine.
    typedef std::list<ast::rRoutine> routine_stack_type;
    routine_stack_type routine_stack_;
    /// Helpers routines to manipulate the frame size stack
    void routine_push(ast::rRoutine f);
    void routine_pop();

    /// The routine currently defined.
    /// Cannot be called if there is none.
    ast::rRoutine routine() const;

    /// The innermost function (not closure).
    /// May return 0.
    ast::rRoutine function() const;
    /// \}

    /// Level of routine nesting.
    unsigned routine_depth_;
    /// Level of scope nesting.
    unsigned scope_depth_;
    /// Local index at the toplevel
    unsigned toplevel_index_;

    /// Register that \a var is bound in any subscope, \a being its
    /// declaration
    void bind(ast::rLocalDeclaration decl);

    /// \return 0 if the variable isn't local, or the depth in
    /// number of nested routines otherwise.
    unsigned routine_depth_get(libport::Symbol name);
    unsigned scope_depth_get(libport::Symbol name);
    ast::rLocalDeclaration decl_get(libport::Symbol name);

    /// Factored method to handle scopes.
    libport::Finally::action_type scope_open(bool set_on_self);
    void scope_close();

    /// Factored method to create updateSlot/setSlot calls.
    ast::rCall changeSlot(const ast::loc& l,
                          const ast::rExp& target,
                          libport::Symbol name,
                          libport::Symbol method,
                          ast::rConstExp value = 0);

    /// Make a lazy from \a arg.
    ast::rExp lazify(ast::rExp arg);

    /// Wether to report errors.
    bool report_errors_;
    /// How many scope to bypass when declaring variables.
    unsigned unscope_;
  };

} // namespace binder

# include <binder/binder.hxx>

#endif // !BINDER_BINDER_HH

/**
 ** \file binder/binder.hh
 ** \brief Definition of binder::Binder.
 */

#ifndef BINDER_BINDER_HH
# define BINDER_BINDER_HH

# include <libport/finally.hh>
# include <list>
# include <map>

# include <ast/all.hh>
# include <ast/analyzer.hh>
# include <ast/error.hxx>
# include <binder/bind.hh>
# include <object/fwd.hh>

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
    typedef object::rObject rObject;
    /// \}

    /// \name Ctor & dtor.
    /// \{
    /// Construct a \c Binder.
    Binder ();

    /// Destroy a Binder.
    virtual ~Binder ();
    /// \}

    /// Import visit from DefaultVisitor.
    using super_type::visit;

  protected:
    CONST_VISITOR_VISIT_NODES(
                              (Assignment)
                              (Call)
                              (CallMsg)
                              (Closure)
                              (Declaration)
                              (Do)
                              (Foreach)
                              (Function)
                              (Scope));

    template <typename Code>
    void handleRoutine(libport::shared_ptr<const Code> code);

    template <typename Node, typename NewNode>
    void link_to_declaration(Node input,
                             NewNode current,
                             const libport::Symbol& name,
                             unsigned depth);

  private:
    /// Actions to perform at exit of the innermost scope.
    typedef std::list<libport::Finally> unbind_type;
    unbind_type unbind_;

    /// Declaration * (routine_depth * scope_depth)
    typedef std::pair<ast::rDeclaration,
                      std::pair<unsigned, unsigned> > binding_type;
    typedef std::list<binding_type> Bindings;
    typedef std::map<libport::Symbol, Bindings> Environment;
    /// Map of currently bound variables
    Environment env_;

    /// Whether to apply setSlot on self
    typedef std::list<bool> set_on_self_type;
    set_on_self_type setOnSelf_;

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
    ast::rFunction function() const;
    /// \}

    /// Level of routine nesting.
    unsigned routine_depth_;
    /// Level of scope nesting.
    unsigned scope_depth_;
    /// Local index at the toplevel
    unsigned toplevel_index_;

    /// Register that \a var is bound in any subscope, \a being its
    /// declaration
    void bind(ast::rDeclaration decl);

    /// \return 0 if the variable isn't local, or the depth in
    /// number of nested routines otherwise.
    unsigned routine_depth_get(const libport::Symbol& name);
    unsigned scope_depth_get(const libport::Symbol& name);
    ast::rDeclaration decl_get(const libport::Symbol& name);

    /// Factored method to handle scopes.
    ast::rExp handleScope(ast::rConstScope scope, bool setOnSelf);

    /// Factored method to create updateSlot/setSlot calls.
    ast::rCall changeSlot(const ast::loc& l,
                          const libport::Symbol& name,
                          const libport::Symbol& method,
                          ast::rConstExp value);

    /// Make a lazy from \a arg
    ast::rExp lazify(ast::rExp arg, const ast::loc& loc);
  };

} // namespace binder

# include <binder/binder.hxx>

#endif // !BINDER_BINDER_HH

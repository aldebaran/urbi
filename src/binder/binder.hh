/**
 ** \file binder/binder.hh
 ** \brief Definition of binder::Binder.
 */

#ifndef BINDER_BINDER_HH
# define BINDER_BINDER_HH

# include <libport/finally.hh>
# include <list>
# include <map>

# include <ast/cloner.hh>
# include <ast/cloner.hxx>
# include <ast/all.hh>
# include <object/fwd.hh>

# include <binder/bind.hh>

namespace binder
{

  /// Ast local variables binder.
  class Binder : public ast::Cloner
  {
    public:
      /// \name Useful shorthands.
      /// \{
      /// Super class type.
      typedef ast::Cloner super_type;
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
      CONST_VISITOR_VISIT_NODES((Assignment)
                                (Call)
                                (Closure)
                                (Declaration)
                                (Do)
                                (Foreach)
                                (Function)
                                (Scope));

      template <typename Code>
      void handleCode(libport::shared_ptr<const Code> code, bool closure);

      template <typename Node, typename NewNode>
      void link_to_declaration(Node input,
                               NewNode current,
                               const libport::Symbol& name,
                               unsigned depth);

    private:

      /// Actions to perform at exit of the most inner scope
      typedef std::list<libport::Finally> unbind_type;
      unbind_type unbind_;

      /// Declaration * depth
      typedef std::pair<ast::rDeclaration, unsigned>
        binding_type;
      typedef std::list<binding_type> Bindings;
      typedef std::map<libport::Symbol, Bindings> Environment;
      /// Map of currently bound variables
      Environment env_;

      /// Whether to apply setSlot on self
      typedef std::list<bool> set_on_self_type;
      set_on_self_type setOnSelf_;

      /// The stack of current number of local variables, and maximum
      /// number of local variable used by the current function.
      typedef std::list<ast::rCode> function_stack_type;
      function_stack_type function_stack_;
      /// Helpers functions to manipulate the frame size stack
      void push_function(ast::rCode f);
      void pop_function();
      ast::rCode function() const;

      /// Level of function imbrication
      unsigned depth_;
      /// Local index at the toplevel
      unsigned toplevel_index_;

      /// Register that \a var is bound in any subscope, \a being its
      /// declaration
      void bind(ast::rDeclaration decl);
      /// \return 0 if the variable isn't local, or the depth in
      /// number of imbriqued function otherwise.
      unsigned depth_get(const libport::Symbol& name);
      ast::rDeclaration decl_get(const libport::Symbol& name);
      /// Factored method to handle scopes.
      ast::rExp handleScope(ast::rConstAbstractScope scope, bool setOnSelf);
      /// Factored method to create updateSlot/setSlot calls.
      ast::rCall changeSlot (const ast::loc& l,
                             const libport::Symbol& name,
                             const libport::Symbol& method,
                             ast::rConstExp value);
      /// Make a lazy from \a arg
      ast::rExp lazify (ast::rExp arg, const ast::loc& loc);
      /// Make a closure from \a arg
      ast::rClosure make_closure(ast::rExp e, const ast::loc& loc);

  };

} // namespace binder

# include <binder/binder.hxx>

#endif // !BINDER_BINDER_HH

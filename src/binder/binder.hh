/**
 ** \file binder/binder.hh
 ** \brief Definition of binder::Binder.
 */

#ifndef BINDER_BINDER_HH
# define BINDER_BINDER_HH

# include <boost/tuple/tuple.hpp>
# include <libport/finally.hh>
# include <list>
# include <map>

# include <ast/cloner.hh>
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
    private:

      /// Actions to perform at exit of the most inner scope
      std::list<libport::Finally> unbind_;

      /// Declaration * depth * local index
      typedef boost::tuple<ast::rConstDeclaration, unsigned, unsigned>
        binding_type;
      typedef std::list<binding_type> Bindings;
      typedef std::map<libport::Symbol, Bindings> Environment;
      /// Map of currently bound variables
      Environment env_;

      /// Whether to apply setSlot on self
      std::list<bool> setOnSelf_;

      /// The stack of current number of local variables, and maximum
      /// number of local variable used by the current function.
      typedef std::list<std::pair<unsigned, unsigned> > locals_size_type;
      locals_size_type locals_size_;
      /// Helpers functions to manipulate the frame size stack
      void push_frame_size();
      void pop_frame_size();
      void inc_frame_size();
      unsigned frame_size() const;

      /// Stack of closed variables
      typedef std::set<libport::Symbol> closed_variables_type;
      typedef std::vector<closed_variables_type> closed_variables_stack_type;
      closed_variables_stack_type closed_variables_stack_;
      /// Helpers to manipulate the closed variables stack
      void push_closed_variables();
      void pop_closed_variables();
      const closed_variables_type& closed_variables() const;

      /// Level of function imbrication
      unsigned depth_;

      /// Register that \a var is bound in any subscope, \a being its
      /// declaration
      void bind(ast::rConstDeclaration decl);
      /// \return 0 if the variable isn't local, or the depth in
      /// number of imbriqued function otherwise.
      unsigned depth_get(const libport::Symbol& name);
      /// \return The local index of the given variable
      /// \precondition The given local variable must exist
      unsigned local_index_get(const libport::Symbol& name);
      /// Factored method to handle scopes.
      ast::rExp handleScope(ast::rConstAbstractScope scope, bool setOnSelf);
      /// Factored method to create updateSlot/setSlot calls.
      ast::rCall changeSlot (const ast::loc& l,
                             const libport::Symbol& name,
                             const libport::Symbol& method,
                             ast::rConstExp value);
  };

} // namespace binder

# include <binder/binder.hxx>

#endif // !BINDER_BINDER_HH

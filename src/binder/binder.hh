/**
 ** \file binder/binder.hh
 ** \brief Definition of binder::Binder.
 */

#ifndef BINDER_BINDER_HH
# define BINDER_BINDER_HH

# include <libport/finally.hh>
# include <map>

# include "ast/default-visitor.hh"
# include "ast/all.hh"
# include "object/fwd.hh"

namespace binder
{

  /// Ast local variables binder.
  class Binder : public ast::DefaultVisitor
  {
    public:
      /// \name Useful shorthands.
      /// \{
      /// Super class type.
      typedef ast::DefaultVisitor super_type;
      /// Import rObject
      typedef object::rObject rObject;
      /// \}

      /// How to bind. Essentialy, in normal mode, unknown variables
      /// are searched in self, while they're search in the context in
      /// context mode.
      enum Mode
      {
        normal,
        context,
      };

      /// \name Ctor & dtor.
      /// \{
      /// Construct a \c Binder.
      /// \param m How to bind
      Binder (Mode m = normal);

      /// Destroy a Binder.
      virtual ~Binder ();
      /// \}

      /// Import visit from DefaultVisitor.
      using super_type::visit;

    protected:
      VISITOR_VISIT_NODES((Call)
                          (Do)
                          (Foreach)
                          (Function)
                          (Scope));
    private:
      typedef std::list<std::pair<ast::Ast*, int> > Bindings;
      typedef std::map<libport::Symbol, Bindings> Environment;
      /// Map of currently bound variables
      Environment env_;
      /// Actions to perform at exit of the most inner scope
      std::list<libport::Finally> unbind_;
      /// Whether to apply setSlot on self
      std::list<bool> setOnSelf_;
      /// Level of function imbrication
      int depth_;
      /// The mode for this binder
      Mode mode_;

      /// Register that \a var is bound in any subscope, \a being its
      /// declaration
      void bind(const libport::Symbol& var, ast::Ast* decl);
      /// Retarget a call according to the mode and whether the \a
      /// variable is set.
      void retarget(ast::Call& call, const libport::Symbol& var);
      /// Retarget a call to getSlot("self")
      void targetSelf(ast::Call& call);
      /// Retarget a call to getSlot("code").context
      void targetContext(ast::Call& call, int depth = 1);
      /// Whether \return 0 If the variable is local, or the depth in
      /// number of imbriqued function otherwise.
      int isLocal(const libport::Symbol& name);
      /// Factored method to handle scopes.
      void handleScope(ast::AbstractScope& scope, bool setOnSelf);
  };

} // namespace binder

# include "binder/binder.hxx"

#endif // !BINDER_BINDER_HH

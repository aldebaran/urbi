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
      typedef object::rObject rObject;
      /// \}

      enum Mode
      {
        normal,
        context,
      };

      /// \name Ctor & dtor.
      /// \{
      /// Construct a \c Binder.
      Binder (Mode m = normal);

      /// Destroy a Binder.
      virtual ~Binder ();
      /// \}

      /// \ name Accessors.
      /// \{
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
      Environment env_;
      // Actions to perform at exit of the most inner scope
      std::list<libport::Finally> unbind_;
      // Whether to apply setSlot on self
      std::list<bool> setOnSelf_;
      // Level of function imbrication
      int depth_;

      void bind(const libport::Symbol& var, ast::Ast* decl);
      void retarget(ast::Call& call, const libport::Symbol& var);
      void targetSelf(ast::Call& call);
      void targetContext(ast::Call& call);
      int isLocal(const libport::Symbol& name);
      void handleScope(ast::AbstractScope& scope, bool setOnSelf);
      Mode mode_;
  };

} // namespace binder

# include "binder/binder.hxx"

#endif // !BINDER_BINDER_HH

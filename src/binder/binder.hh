/**
 ** \file binder/binder.hh
 ** \brief Definition of binder::Binder.
 */

#ifndef BINDER_BINDER_HH
# define BINDER_BINDER_HH

# include <libport/finally.hh>
# include <map>

# include <ast/cloner.hh>
# include "ast/all.hh"
# include "object/fwd.hh"

# include "binder/bind.hh"

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
      typedef std::list<std::pair<ast::rConstAst, int> > Bindings;
      typedef std::map<libport::Symbol, Bindings> Environment;
      /// Map of currently bound variables
      Environment env_;
      /// Actions to perform at exit of the most inner scope
      std::list<libport::Finally> unbind_;
      /// Whether to apply setSlot on self
      std::list<bool> setOnSelf_;
      /// Level of function imbrication
      int depth_;

      /// Register that \a var is bound in any subscope, \a being its
      /// declaration
      void bind(const libport::Symbol& var, ast::rConstAst decl);
      /// Retarget a call according to whether the \a variable is set.
      void retarget(ast::rCall call, const libport::Symbol& var);
      /// Retarget a call to getSlot("self")
      void targetSelf(ast::rCall call);
      /// Retarget a call to getSlot("code").context
      void targetContext(ast::rCall call, int depth = 1);
      /// Whether \return 0 If the variable is local, or the depth in
      /// number of imbriqued function otherwise.
      int isLocal(const libport::Symbol& name);
      /// Factored method to handle scopes.
      ast::rExp handleScope(ast::rConstAbstractScope scope, bool setOnSelf);
  };

} // namespace binder

# include "binder/binder.hxx"

#endif // !BINDER_BINDER_HH

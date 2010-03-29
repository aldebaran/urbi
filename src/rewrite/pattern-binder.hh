/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef REWRITE_PATTERN_BINDER_HH
# define REWRITE_PATTERN_BINDER_HH

# include <memory>
# include <ast/fwd.hh>
# include <ast/cloner.hh>

namespace rewrite
{
  class PatternBinder: public ast::Cloner
  {
  public:
    typedef ast::Cloner super_type;
    PatternBinder(ast::rLValue pattern,
                  const ast::loc& loc);
    ast::rPipe bindings_get() const;
    ast::rExp value_get();

  protected:
    using super_type::visit;
    CONST_VISITOR_VISIT_NODES(
      (Binding)
      );

  private:
    libport::Symbol next_name();
    ast::rExp to_binding(ast::rConstExp original);
    void bind(ast::rConstLValue what, bool decl);
    ast::rPipe bindings_;
    ast::rPipe declarations_;
    ast::rLValue pattern_;
    int i_;
    std::auto_ptr<ast::Factory> factory_;
  };
}

#endif

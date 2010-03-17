/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_REWRITE_CALL_EXTRACTOR_HH
# define URBI_REWRITE_CALL_EXTRACTOR_HH

# include <vector>

# include <ast/transformer.hh>

namespace rewrite
{
  class CallExtractor: public ast::Transformer
  {
  public:
    typedef Transformer super_type;

    CallExtractor();

    using super_type::visit;
    VISITOR_VISIT_NODES((Call));

    const ast::exps_type& declarations_get()
    {
      return decls_;
    }

    const ast::exps_type& changed_get()
    {
      return changed_;
    }

  private:
    unsigned idx_;
    ast::exps_type decls_;
    ast::exps_type changed_;
  };
}

#endif

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

# include <libport/attributes.hh>

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

    ATTRIBUTE_R(ast::exps_type, declarations);
    ATTRIBUTE_R(ast::exps_type, changed);
    ATTRIBUTE  (unsigned, idx);
  };
}

#endif

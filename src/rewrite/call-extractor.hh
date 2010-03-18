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
  /* Analyze a condition and deduce which events to listen to know
   * when to recompute the condition.
   * Result: a first evaluation of the condition.
   */
  class CallExtractor: public ast::Transformer
  {
  public:
    typedef Transformer super_type;

    CallExtractor();

    using super_type::visit;
    VISITOR_VISIT_NODES((Call));

    /// Step-by-step evaluation of the condition. Include these before using res.
    ATTRIBUTE_R(ast::exps_type, declarations);
    /// The events signaling a potential change in the condition evaluation
    ATTRIBUTE_R(ast::exps_type, changed);
    ATTRIBUTE  (unsigned, idx);
  };
}

#endif

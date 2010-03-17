/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/format.hh>
#include <object/symbols.hh>

#include <rewrite/call-extractor.hh>

namespace rewrite
{
  CallExtractor::CallExtractor()
    : idx_(0)
  {
    result_ = 0;
  }

  void
  CallExtractor::visit(ast::Call* node)
  {
    const ast::loc& loc = node->location_get();

    transform(node->target_get());
    transform_collection(node->arguments_get());
    ast::rCall res = new ast::Call(loc, 0, new ast::Implicit(loc), libport::Symbol(libport::format("$%s", idx_++)));
    decls_.push_back(new ast::Declaration(loc, res, node));
    if (!node->arguments_get())
    {
      changed_.push_back(new ast::Call(loc, 0, node, SYMBOL(changed)));
      changed_.push_back(new ast::Property(loc, node, SYMBOL(changed)));
    }

//    result_ = new ast::Call(node->location_get(), node->arguments_get(), node->target_get(), libport::Symbol(libport::format("$%s", idx_++)));
    result_ = res;
  }
}

#include <libport/foreach.hh>

#include "runner/tag.hh"

namespace runner
{
  Tag::Tag ()
  {
  }

  Tag::~Tag ()
  {
    foreach (Tag* parent, parents_)
      parent->unregister_dependent (*this);
  }

  void
  Tag::register_dependent (Tag& child)
  {
    dependents_.insert (&child);
    child.parents_.insert (this);
  }

  void
  Tag::unregister_dependent (Tag& child)
  {
    dependents_.erase (&child);
    child.parents_.erase (this);
  }

  void
  Tag::copy_parents (const Tag& model)
  {
    // Unregister existing parents
    foreach (Tag* parent, parents_)
      parent->unregister_dependent (*this);
    // Copy parents from model
    foreach (Tag* parent, model.parents_)
      parent->register_dependent (*this);
  }

  void
  Tag::propagate (operation_type operation)
  {
    bool starting = operation == unfreeze || operation == resume;

    if (starting)
      act (operation);
    foreach (Tag* d, dependents_)
      d->propagate (operation);
    if (!starting)
      act (operation);
  }

  void
  Tag::act (operation_type)
  {
    // Do nothing.
  }

} // namespace runner

/**
 ** \file object/tag-class.hh
 ** \brief Definition of the URBI object tag.
 */

#ifndef OBJECT_TAG_CLASS_HH
# define OBJECT_TAG_CLASS_HH

# include "object/fwd.hh"
# include "scheduler/fwd.hh"

namespace object
{
  extern rObject tag_class;

  /// Initialize the Task class.
  void tag_class_initialize ();

  /// Extract the tag object
  scheduler::rTag extract_tag (const rObject&);

  /// Create an Urbi tag from a scheduler tag.
  rObject create_tag(scheduler::rTag);
}; // namespace object

#endif // !OBJECT_TAG_CLASS_HH

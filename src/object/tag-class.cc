/**
 ** \file object/tag-class.cc
 ** \brief Creation of the URBI object tag.
 */

#include <boost/any.hpp>

#include "object/tag-class.hh"

#include "object/alien.hh"
#include "object/atom.hh"
#include "object/object.hh"

#include "runner/runner.hh"
#include "scheduler/tag.hh"

namespace object
{
  rObject tag_class;

  /*-----------------.
  | Tag primitives.  |
  `-----------------*/

  scheduler::rTag
  extract_tag (const rObject& o)
  {
    return unbox (scheduler::rTag, o->slot_get (SYMBOL (tag)));
  }

  static rObject
  tag_class_init (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT_RANGE (1, 2);

    // If a parent is specified and this is not Tag, then the
    // underlying scheduler tag will get its underlying object
    // as parent.
    scheduler::rTag mytag;
    if (args.size () == 2 && args[1] != tag_class)
      mytag = new scheduler::Tag (extract_tag (args[1]));
    else
      mytag = new scheduler::Tag;
    args[0]->slot_set (SYMBOL (tag), box (scheduler::rTag, mytag));

    return args[0];
  }

#define TAG_ACTION(Action)					\
  static rObject						\
  tag_class_ ## Action (runner::Runner& r, objects_type args)	\
  {								\
    CHECK_ARG_COUNT (1);					\
    extract_tag (args[0])->Action (r);				\
    return void_class;						\
  }
  TAG_ACTION(block)
  TAG_ACTION(freeze)
  TAG_ACTION(stop)
  TAG_ACTION(unblock)
  TAG_ACTION(unfreeze)
#undef TAG_ACTION

  static rObject
  tag_class_asString (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    // FIXME: Decide if and how tags are named.
    return new String (libport::Symbol ("<tag>"));
  }

  void
  tag_class_initialize ()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(tag, Name)
    DECLARE (asString);
    DECLARE (block);
    DECLARE (freeze);
    DECLARE (init);
    DECLARE (stop);
    DECLARE (unblock);
    DECLARE (unfreeze);
#undef DECLARE
  }

}; // namespace object

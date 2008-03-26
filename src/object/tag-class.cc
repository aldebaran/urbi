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
    CHECK_ARG_COUNT_RANGE (1, 3);
    libport::Symbol tag_short_name;

    if (args.size () > 1)
    {
      FETCH_ARG (1, String);
      tag_short_name = arg1->value_get ();
    }
    else
      tag_short_name = libport::Symbol::fresh (SYMBOL (tag));

    // If a parent is specified and this is not Tag, then the
    // underlying scheduler tag will get its underlying object
    // as parent.
    scheduler::rTag mytag;
    if (args.size () == 3 && args[1] != tag_class)
      mytag = new scheduler::Tag (extract_tag (args[2]),
				  tag_short_name);
    else
      mytag = new scheduler::Tag (tag_short_name);
    args[0]->slot_set (SYMBOL (tag), box (scheduler::rTag, mytag));

    return args[0];
  }

  static rObject
  tag_class_name (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);

    return new String (extract_tag (args[0])->name_get ());
  }

#define TAG_ACTION(Action, Yield)				\
  static rObject						\
  tag_class_ ## Action (runner::Runner& r, objects_type args)	\
  {								\
    CHECK_ARG_COUNT (1);					\
    scheduler::rTag self = extract_tag (args[0]);		\
    self->Action (r, self);					\
    if (Yield && (r.frozen () || r.blocked ()))			\
      r.yield ();						\
    return void_class;						\
  }
  TAG_ACTION(block, true)
  TAG_ACTION(freeze, true)
  TAG_ACTION(stop, true)
  TAG_ACTION(unblock, false)
  TAG_ACTION(unfreeze, false)
#undef TAG_ACTION

  void
  tag_class_initialize ()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(tag, Name)
    DECLARE (block);
    DECLARE (freeze);
    DECLARE (init);
    DECLARE (name);
    DECLARE (stop);
    DECLARE (unblock);
    DECLARE (unfreeze);
#undef DECLARE
  }

}; // namespace object

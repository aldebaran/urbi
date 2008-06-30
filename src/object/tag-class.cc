/**
 ** \file object/tag-class.cc
 ** \brief Creation of the URBI object tag.
 */

#include <boost/any.hpp>

#include <object/tag-class.hh>

#include <object/alien.hh>
#include <object/atom.hh>
#include <object/object.hh>
#include <object/string-class.hh>

#include <runner/runner.hh>
#include <scheduler/tag.hh>

namespace object
{

  rObject tag_class;

  Tag::Tag()
    : value_(0)
  {
    proto_add(tag_class);
  }

  Tag::Tag(const value_type& value)
    : value_(value)
  {
    proto_add(tag_class);
  }

  Tag::Tag(rTag model)
    : value_(model->value_)
  {
    proto_add(tag_class);
  }

  const Tag::value_type&
  Tag::value_get() const
  {
    return value_;
  }

  void
  Tag::block(runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT_RANGE(0, 1);
    rObject payload = boost::any_cast<rObject>(args.empty() ? void_class : args[0]);
    value_->block(r.scheduler_get(), payload);
  }

  void
  Tag::freeze(runner::Runner& r)
  {
    value_->freeze(r.scheduler_get());
    if (r.frozen())
      r.yield();
  }

  rString
  Tag::name()
  {
    return new String(value_->name_get());
  }

  rTag
  Tag::_new(objects_type args)
  {
    CHECK_ARG_COUNT_RANGE(1, 2);
    libport::Symbol tag_short_name;

    if (args.size() > 1)
    {
      type_check<String>(args[1], SYMBOL(new));
      tag_short_name = args[1]->as<String>()->value_get();
    }
    else
      tag_short_name = libport::Symbol::fresh(SYMBOL(tag));

    rTag res = new Tag(args[0] == tag_class ?
		       new scheduler::Tag(tag_short_name) :
		       extract_tag(args[0]));
    return res;
  }

  void
  Tag::stop(runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT_RANGE(0, 1);
    rObject payload = boost::any_cast<rObject>(args.empty() ? void_class : args[0]);
    value_->stop(r.scheduler_get(), payload);
  }

  void
  Tag::unblock(runner::Runner& r)
  {
    value_->unblock(r.scheduler_get());
  }

  void
  Tag::unfreeze(runner::Runner& r)
  {
    value_->unfreeze(r.scheduler_get());
  }

  void
  Tag::initialize(CxxObject::Binder<Tag>& bind)
  {
    bind(SYMBOL(block), &Tag::block);
    bind(SYMBOL(freeze), &Tag::freeze);
    bind(SYMBOL(name), &Tag::name);
    bind(SYMBOL(new), &Tag::_new);
    bind(SYMBOL(stop), &Tag::stop);
    bind(SYMBOL(unblock), &Tag::unblock);
    bind(SYMBOL(unfreeze), &Tag::unfreeze);
  }

  bool Tag::tag_added = CxxObject::add<Tag>("Tag", tag_class);
  const std::string Tag::type_name = "Tag";
  std::string Tag::type_name_get() const
  {
    return type_name;
  }

  scheduler::rTag
  extract_tag(const rObject& o)
  {
    type_check<Tag>(o, SYMBOL(extract_tag));
    return o->as<Tag>()->value_get();
  }

}; // namespace object

/**
 ** \file object/tag-class.hh
 ** \brief Definition of the URBI object tag.
 */

#ifndef OBJECT_TAG_CLASS_HH
# define OBJECT_TAG_CLASS_HH

# include <object/cxx-object.hh>
# include <object/fwd.hh>
# include <scheduler/fwd.hh>

namespace object
{
  extern rObject tag_class;

  class Tag : public object::CxxObject
  {
  public:
    typedef scheduler::rTag value_type;

    Tag();
    Tag(const value_type& value);
    Tag(rTag model);
    const value_type& value_get() const;

    void block(runner::Runner&, objects_type);
    static rTag _new(objects_type);
    void freeze(runner::Runner&);
    rString name();
    void stop(runner::Runner&, objects_type);
    void unblock(runner::Runner&);
    void unfreeze(runner::Runner&);

    static void initialize(CxxObject::Binder<Tag>& bind);
    static const std::string type_name;
    static bool tag_added;
    virtual std::string type_name_get() const;

  private:
    value_type value_;
  };

  scheduler::rTag
  extract_tag(const rObject& o);

} // namespace object

#endif // !OBJECT_TAG_CLASS_HH

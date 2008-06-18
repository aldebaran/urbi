/**
 ** \file object/list-class.cc
 ** \brief Creation of the URBI object list.
 */

#include <boost/assign.hpp>
#include <boost/bind.hpp>

#include <libport/foreach.hh>
#include <libport/ufloat.hh>

#include <kernel/userver.hh>
#include <object/code-class.hh>
#include <object/float-class.hh>
#include <object/list-class.hh>
#include <object/atom.hh>
#include <object/object.hh>
#include <object/primitives.hh>
#include <runner/runner.hh>

using namespace boost::assign;

namespace object
{
  rObject list_class;

  List::List()
  {
    proto_add(list_class);
  }

  List::List(const value_type& value)
    : content_(value)
  {
    proto_add(list_class);
  }

  List::List(rList model)
    : content_(model->content_)
  {
    proto_add(list_class);
  }

  const List::value_type& List::value_get() const
  {
    return content_;
  }

  List::value_type& List::value_get()
  {
    return content_;
  }

#define CHECK_NON_EMPTY(Name)                                           \
  if (content_.empty ())                                                \
    throw PrimitiveError						\
      (Name, "operation cannot be applied onto empty list")

  rList List::tail()
  {
    CHECK_NON_EMPTY("tail");
    value_type res = content_;
    res.pop_front();
    return new List(res);
  }

  rObject List::operator[](rFloat idx)
  {
    unsigned i = idx->to_int(SYMBOL(nth));
    if (i >= content_.size())
      throw PrimitiveError("nth", "invalid index");
    return content_[i];
  }

  rFloat List::size()
  {
    return new Float(content_.size());
  }

  rList List::remove_by_id(rObject elt)
  {
    value_type::iterator it = content_.begin();
    while (it != content_.end())
      if (*it == elt)
        it = content_.erase(it);
      else
        ++it;
    return this;
  }

  rList List::operator+=(rList rhs)
  {
    // Copy the list to make a += a work
    value_type other = rhs->value_get();
    foreach (const rObject& o, other)
      content_.push_back(o);
    return this;
  }

  rList List::operator+(rList rhs)
  {
    rList res = new List(this);
    *res += rhs;
    return res;
  }

  /// Binary predicate used to sort lists.
  static bool
  compareListItems (runner::Runner& r, rObject a, rObject b)
  {
    return is_true(urbi_call(r, a, SYMBOL(LT), b));
  }

  rList List::sort(runner::Runner& r)
  {
    std::list<rObject> s;
    foreach(const rObject& o, content_)
      s.push_back(o);
    s.sort(boost::bind(compareListItems, boost::ref(r), _1, _2));

    List::value_type res;
    foreach(const rObject& o, s)
      res.push_back(o);
    return new List(res);
  }

  void
  List::each(runner::Runner& r, rObject f)
  {
    foreach(const rObject& o, content_)
      r.apply(f, SYMBOL(each), list_of (f) (o));
  }

#define BOUNCE(Name, Ret, Arg, Check)                           \
  IF(Ret, rObject, rList) List::Name(WHEN(Arg, rObject arg))    \
  {                                                             \
    WHEN(Check, CHECK_NON_EMPTY(#Name));                        \
    WHEN(Ret, return) content_.Name(WHEN(Arg, arg));            \
    return this;                                                \
  }

  BOUNCE(back,       true,  false, true );
  BOUNCE(clear,      false, false, false);
  BOUNCE(front,      true,  false, true );
  BOUNCE(pop_back,   false, false, true );
  BOUNCE(pop_front,  false, false, true );
  BOUNCE(push_back,  false, true,  false);
  BOUNCE(push_front, false, true,  false);

#undef BOUNCE

  void List::initialize(CxxObject::Binder<List>& bind)
  {
    bind(SYMBOL(back),         &List::back        );
    bind(SYMBOL(clear),        &List::clear       );
    bind(SYMBOL(each),         &List::each        );
    bind(SYMBOL(front),        &List::front       );
    bind(SYMBOL(PLUS),         &List::operator+   );
    bind(SYMBOL(PLUS_EQ),      &List::operator+=  );
    bind(SYMBOL(nth),          &List::operator[]  );
    bind(SYMBOL(push_back),    &List::push_back   );
    bind(SYMBOL(push_front),   &List::push_front  );
    bind(SYMBOL(pop_back),     &List::pop_back    );
    bind(SYMBOL(pop_front),    &List::pop_front   );
    bind(SYMBOL(removeById),   &List::remove_by_id);
    bind(SYMBOL(size),         &List::size        );
    bind(SYMBOL(sort),         &List::sort        );
    bind(SYMBOL(tail),         &List::tail        );
  }

  std::string List::type_name_get() const
  {
    return type_name;
  }

  bool List::list_added =
    CxxObject::add<List>("List", list_class);
  const std::string List::type_name = "List";


} // namespace object

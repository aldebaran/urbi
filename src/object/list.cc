/**
 ** \file object/list-class.cc
 ** \brief Creation of the URBI object list.
 */

#include <boost/bind.hpp>

#include <libport/foreach.hh>
#include <libport/lexical-cast.hh>
#include <libport/finally.hh>
#include <libport/ufloat.hh>

#include <kernel/userver.hh>
#include <object/code.hh>
#include <object/float.hh>
#include <object/list.hh>
#include <object/object.hh>
#include <object/primitives.hh>

#include <runner/raise.hh>
#include <runner/runner.hh>
#include <runner/interpreter.hh>

namespace object
{
  List::List()
  {
    proto_add(proto ? proto : object_class);
  }

  List::List(const value_type& value)
    : content_(value)
  {
    proto_add(List::proto);
  }

  List::List(const rList& model)
    : content_(model->content_)
  {
    proto_add(List::proto);
  }

  const List::value_type& List::value_get() const
  {
    return content_;
  }

  List::value_type& List::value_get()
  {
    return content_;
  }

#define CHECK_NON_EMPTY(Name)			\
  do {						\
    if (content_.empty())			\
      runner::raise_primitive_error		\
	("cannot be applied onto empty list");	\
  } while (0)

  rList
  List::tail()
  {
    CHECK_NON_EMPTY(tail);
    value_type res = content_;
    libport::pop_front(res);
    return new List(res);
  }

  List::size_type
  List::index(const rFloat& idx) const
  {
    int i = idx->to_int("invalid index: %s");
    if (i < 0)
      i += content_.size();
    if (i < 0 || content_.size() <= static_cast<size_type>(i))
      runner::raise_primitive_error("invalid index: " +
				    string_cast(idx->value_get()));
    return static_cast<size_type>(i);
  }

  rObject List::operator[](const rFloat& idx)
  {
    return content_[index(idx)];
  }

  rObject List::set(const rFloat& idx, const rObject& val)
  {
    return content_[index(idx)] = val;
  }

  rFloat List::size()
  {
    return new Float(content_.size());
  }

  rList List::remove_by_id(const rObject& elt)
  {
    value_type::iterator it = content_.begin();
    while (it != content_.end())
      if (*it == elt)
        it = content_.erase(it);
      else
        ++it;
    return this;
  }

  rList List::operator+=(const rList& rhs)
  {
    // Copy the list to make a += a work
    value_type other = rhs->value_get();
    foreach (const rObject& o, other)
      content_.push_back(o);
    return this;
  }

  rList List::operator+(const rList& rhs)
  {
    rList res = new List(this);
    *res += rhs;
    return res;
  }

  rList List::operator*(unsigned int times)
  {
    List::value_type res;
    unsigned int s = content_.size();
    for (unsigned int i = 0; i < times; ++i)
      for (unsigned int j = 0; j < s; ++j)
	res.push_back(content_[j]);

    return new List(res);
  }

  /// Binary predicates used to sort lists.
  static bool
  compareListItems (const rObject& a, const rObject& b)
  {
    return is_true(a->call(SYMBOL(LT), b));
  }
  static bool
  compareListItemsLambda (const rObject& f, const rObject& l,
                          const rObject& a, const rObject& b)
  {
    rExecutable fun = f.unsafe_cast<Executable>();
    if (!fun)
    {
      type_check<Code>(f);
      unreached();
    }
    objects_type args;
    args << l;
    args << a;
    args << b;
    return is_true((*fun)(args));
  }

  List::value_type List::sort()
  {
    value_type s(content_);
    std::sort(s.begin(), s.end(),
              boost::bind(compareListItems, _1, _2));
    return s;
  }

  List::value_type List::sort(rObject f)
  {
    value_type s(content_);
    std::sort(s.begin(), s.end(),
              boost::bind(compareListItemsLambda, f, this, _1, _2));
    return s;
  }

  void
  List::each_common(const rObject& f, bool yielding)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();

    // Beware of iterations that modify the list in place: make a
    // copy.
    bool must_yield = false;
    foreach (const rObject& o, value_type(content_))
    {
      if (must_yield)
	r.yield();
      must_yield = yielding;
      objects_type args;
      args.push_back(f);
      args.push_back(o);
      r.apply(f, SYMBOL(each), args);
    }
  }

  void
  List::each(const rObject& f)
  {
    each_common(f, true);
  }

  void
  List::each_pipe(const rObject& f)
  {
    each_common(f, false);
  }

  void
  List::each_and(const rObject& f)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();

    libport::Finally finally;
    sched::jobs_type jobs;

    // Beware of iterations that modify the list in place: make a
    // copy.
    value_type l(content_);
    foreach (const rObject& o, l)
    {
      object::objects_type args;
      args.push_back(o);
      sched::rJob job =
        new runner::Interpreter(dynamic_cast<runner::Interpreter&>(r),
                                f, SYMBOL(each), args);
      r.register_child(job, finally);
      job->start_job();
      jobs.push_back(job);
    }

    try
    {
      r.yield_until_terminated(jobs);
    }
    catch (const sched::ChildException& ce)
    {
      ce.rethrow_child_exception();
    }
  }
  rList
  List::insertFront(const rObject& o)
  {
    libport::push_front(content_, o);
    return this;
  }

#define BOUNCE(Name, Bounce, Ret, Arg, Check)                   \
  IF(Ret, rObject, rList)                                       \
  List::Name(WHEN(Arg, const rObject& arg))			\
  {                                                             \
    WHEN(Check, CHECK_NON_EMPTY(Name));                         \
    WHEN(Ret, return) content_.Bounce(WHEN(Arg, arg));          \
    return this;                                                \
  }

  BOUNCE(back,          back,           true,  false, true );
  BOUNCE(clear,         clear,          false, false, false);
  BOUNCE(front,         front,          true,  false, true );
  BOUNCE(insertBack,    push_back,      false, true,  false);

#undef BOUNCE

  rList
  List::insert(const rFloat& idx, const rObject& elt)
  {
    size_type i = index(idx);
    value_type::iterator b = content_.begin();
    while(i--)
      ++b;
    content_.insert(b, elt);
    return this;
  }

  rObject
  List::removeBack()
  {
    CHECK_NON_EMPTY(pop_back);
    rObject res = content_.back();
    content_.pop_back();
    return res;
  }

  rObject
  List::removeFront()
  {
    CHECK_NON_EMPTY(pop_front);
    rObject res = content_.front();
    libport::pop_front(content_);
    return res;
  }

  rList List::reverse()
  {
    value_type res;
    rforeach (const rObject& obj, content_)
      res.push_back(obj);
    return new List(res);
  }

  OVERLOAD_2(sort_bouncer, 1,
             (List::value_type (List::*) ()) &List::sort,
             (List::value_type (List::*) (rObject f)) &List::sort)

  void List::initialize(CxxObject::Binder<List>& bind)
  {
    bind(SYMBOL(back),           &List::back            );
    bind(SYMBOL(clear),          &List::clear           );
    bind(SYMBOL(each),           &List::each            );
    bind(SYMBOL(each_AMPERSAND), &List::each_and        );
    bind(SYMBOL(each_PIPE),      &List::each_pipe       );
    bind(SYMBOL(front),          &List::front           );
    bind(SYMBOL(SBL_SBR),        &List::operator[]      );
    bind(SYMBOL(SBL_SBR_EQ),     &List::set             );
    bind(SYMBOL(PLUS),           &List::operator+       );
    bind(SYMBOL(PLUS_EQ),        &List::operator+=      );
    bind(SYMBOL(insert),         &List::insert          );
    bind(SYMBOL(insertBack),     &List::insertBack      );
    bind(SYMBOL(insertFront),    &List::insertFront     );
    bind(SYMBOL(removeBack),     &List::removeBack      );
    bind(SYMBOL(removeFront),    &List::removeFront     );
    bind(SYMBOL(removeById),     &List::remove_by_id    );
    bind(SYMBOL(reverse),        &List::reverse         );
    bind(SYMBOL(size),           &List::size            );
    bind(SYMBOL(sort),           &sort_bouncer          );
    bind(SYMBOL(STAR),           &List::operator*       );
    bind(SYMBOL(tail),           &List::tail            );
  }

  URBI_CXX_OBJECT_REGISTER(List);

  rObject
  List::proto_make()
  {
    return new List();
  }

} // namespace object

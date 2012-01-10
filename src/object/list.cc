/*
 * Copyright (C) 2007-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/list.cc
 ** \brief Creation of the Urbi object list.
 */

#include <libport/bind.hh>

#include <libport/foreach.hh>
#include <libport/lexical-cast.hh>
#include <libport/finally.hh>
#include <libport/ufloat.hh>

#include <urbi/kernel/userver.hh>

#include <object/cxx-helper.hh>
#include <urbi/object/symbols.hh>

#include <object/code.hh>
#include <urbi/object/event.hh>
#include <urbi/object/float.hh>
#include <urbi/object/hash.hh>
#include <urbi/object/list.hh>
#include <urbi/object/object.hh>

#include <urbi/runner/raise.hh>
#include <runner/runner.hh>
#include <runner/interpreter.hh>

namespace urbi
{
  namespace object
  {
    List::List()
      : sizeChanged_(0)
      , contentChanged_(0)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    List::List(const value_type& value)
      : content_(value)
      , sizeChanged_(0)
      , contentChanged_(0)
    {
      proto_add(proto);
    }

    List::List(const rList& model)
      : content_(model->content_)
      , sizeChanged_(0)
      , contentChanged_(0)
    {
      proto_add(model);
    }

    URBI_CXX_OBJECT_INIT(List)
      : sizeChanged_(0)
      , contentChanged_(0)
    {
      bind(SYMBOL(sort),
           static_cast<List::value_type (List::*)()>(&List::sort));
      bind(SYMBOL(sort),
           static_cast<List::value_type (List::*)(rObject)>
             (&List::sort));

#define DECLARE(Name, Function)                 \
      bind(SYMBOL_(Name), &List::Function)

      DECLARE(asBool,         as_bool         );
      DECLARE(asString,       as_string       );
      DECLARE(back,           back            );
      DECLARE(clear,          clear           );
      DECLARE(each,           each            );
      DECLARE(each_AMPERSAND, each_and        );
      DECLARE(each_PIPE,      each_pipe       );
      DECLARE(eachi,          eachi           );
      DECLARE(empty,          empty           );
      DECLARE(front,          front           );
      DECLARE(hash,           hash            );
      DECLARE(SBL_SBR,        operator[]      );
      DECLARE(SBL_SBR_EQ,     set             );
      DECLARE(PLUS,           operator+       );
      DECLARE(PLUS_EQ,        operator+=      );
      DECLARE(EQ_EQ,          operator==      );
      DECLARE(insert,         insert          );
      DECLARE(insertBack,     insertBack      );
      DECLARE(insertFront,    insertFront     );
      DECLARE(removeBack,     removeBack      );
      DECLARE(removeFront,    removeFront     );
      DECLARE(removeById,     remove_by_id    );
      DECLARE(reverse,        reverse         );
      DECLARE(size,           size            );
      DECLARE(STAR,           operator*       );
      DECLARE(tail,           tail            );

#undef DECLARE
    }

    const List::value_type& List::value_get() const
    {
      return content_;
    }

    List::value_type& List::value_get()
    {
      return content_;
    }

#define CHECK_NON_EMPTY(Name)                           \
    do {                                                \
      if (content_.empty())                             \
        RAISE("cannot be applied onto empty list");	\
    } while (0)

    rList
    List::tail() const
    {
      URBI_AT_HOOK(contentChanged);
      CHECK_NON_EMPTY(tail);
      value_type res = content_;
      res.pop_front();
      return new List(res);
    }

    bool
    List::as_bool() const
    {
      return !empty();
    }

    bool
    List::empty() const
    {
      URBI_AT_HOOK(sizeChanged);
      return content_.empty();
    }

    List::size_type
    List::index(const rFloat& idx) const
    {
      int i = idx->to_int_type("invalid index: %s");
      if (i < 0)
        i += content_.size();
      if (i < 0 || content_.size() <= static_cast<size_type>(i))
        FRAISE("invalid index: %s", idx->value_get());
      return static_cast<size_type>(i);
    }

    rObject List::operator[](const rFloat& idx)
    {
      URBI_AT_HOOK(contentChanged);
      return content_[index(idx)];
    }

    rObject List::set(const rFloat& idx, const rObject& val)
    {
      content_[index(idx)] = val;
      contentChanged();
      return val;
    }

    List::size_type
    List::size() const
    {
      URBI_AT_HOOK(sizeChanged);
      return content_.size();
    }

    rList List::remove_by_id(const rObject& elt)
    {
      bool mutated = false;
      value_type::iterator it = content_.begin();
      while (it != content_.end())
        if (*it == elt)
        {
          mutated = true;
          it = content_.erase(it);
        }
        else
          ++it;
      if (mutated)
      {
        contentChanged();
        sizeChanged();
      }
      return this;
    }

    bool
    List::operator == (List* rhs) const
    {
      URBI_AT_HOOK(contentChanged);
      try
      {
        if (rhs->size() != size())
          return false;
        for (unsigned i = 0; i < content_.size(); ++i)
          if (!from_urbi<bool>(content_[i]->call(SYMBOL(EQ_EQ),
                                                 rhs->content_[i])))
            return false;
      }
      catch (UrbiException& e)
      {
        return false;
      }
      return true;
    }

    rList List::operator+=(const rList& rhs)
    {
      URBI_AT_HOOK(contentChanged);
      // Copy the list to make a += a work
      value_type other = rhs->value_get();
      if (!other.empty())
      {
        foreach (const rObject& o, other)
          content_.push_back(o);
        contentChanged();
        sizeChanged();
      }
      return this;
    }

    rList List::operator+(const rList& rhs)
    {
      URBI_AT_HOOK(contentChanged);
      rList res = new List(this->value_get());
      *res += rhs;
      return res;
    }

    rList List::operator*(unsigned int times) const
    {
      URBI_AT_HOOK(contentChanged);
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
      return a->call(SYMBOL(LT), b)->as_bool();
    }
    static bool
    compareListItemsLambda (const rObject& f, const rObject& l,
                            const rObject& a, const rObject& b)
    {
      rExecutable fun = from_urbi<rExecutable>(f);

      objects_type args;
      args << l << a << b;
      return (*fun)(args)->as_bool();
    }

    List::value_type List::sort()
    {
      URBI_AT_HOOK(contentChanged);
      value_type s(content_);
      std::sort(s.begin(), s.end(),
                boost::bind(compareListItems, _1, _2));
      return s;
    }

    List::value_type List::sort(rObject f)
    {
      URBI_AT_HOOK(contentChanged);
      value_type s(content_);
      std::sort(s.begin(), s.end(),
                boost::bind(compareListItemsLambda, f, this, _1, _2));
      return s;
    }

    void
    List::each_common(const rObject& f, bool yielding, bool idx)
    {
      URBI_AT_HOOK(contentChanged);
      runner::Runner& r = ::kernel::runner();

      bool must_yield = false;
      int i = 0;
      // Beware of iterations that modify the list in place: make a
      // copy.
      foreach (const rObject& o, value_type(content_))
      {
        if (must_yield)
          r.yield();
        must_yield = yielding;
        objects_type args;
        args << f << o;
        if (idx)
          args << to_urbi(i++);
        r.apply(f, SYMBOL(each), args);
      }
    }

    void
    List::each(const rObject& f)
    {
      each_common(f, true, false);
    }

    void
    List::eachi(const rObject& f)
    {
      each_common(f, true, true);
    }

    void
    List::each_pipe(const rObject& f)
    {
      each_common(f, false, false);
    }

    void
    List::each_and(const rObject& f)
    {
      URBI_AT_HOOK(contentChanged);
      runner::Runner& r = ::kernel::runner();

      // Beware of iterations that modify the list in place: make a
      // copy.
      value_type l(content_);

      sched::Job::Collector collector(&r, l.size());

      foreach (const rObject& o, l)
      {
        object::objects_type args;
        args.push_back(o);
        sched::rJob job =
          new runner::Interpreter(dynamic_cast<runner::Interpreter&>(r),
                                  f, SYMBOL(each_AMPERSAND), args);
        r.register_child(job, collector);
        job->start_job();
      }

      try
      {
        r.yield_until_terminated(collector);
      }
      catch (const sched::ChildException& ce)
      {
        ce.rethrow_child_exception();
      }
    }

    rList
    List::insertFront(const rObject& o)
    {
      content_.push_front(o);
      contentChanged();
      sizeChanged();
      return this;
    }

    rList
    List::insertBack(const rObject& o)
    {
      content_.push_back(o);
      contentChanged();
      sizeChanged();
      return this;
    }

#define BOUNCE(Name, Bounce, Ret, Check, Mutate)                \
    IF(Ret, rObject, rList)                                     \
    List::Name()                                                \
    {                                                           \
      URBI_AT_HOOK(contentChanged);                                            \
      WHEN(Check, CHECK_NON_EMPTY(Name));                       \
      WHEN(Ret, return) content_.Bounce();                      \
      WHEN(Mutate, contentChanged(); sizeChanged());            \
      return this;                                              \
    }

    BOUNCE(back,          back,           true,  true , false);
    BOUNCE(clear,         clear,          false, false, true);
    BOUNCE(front,         front,          true,  true , false);

#undef BOUNCE

    rList
    List::insert(const rFloat& idx, const rObject& elt)
    {
      content_.insert(boost::next(content_.begin(), index(idx)), elt);
      contentChanged();
      sizeChanged();
      return this;
    }

    rHash
    List::hash() const
    {
      URBI_AT_HOOK(contentChanged);
      rHash res = new Hash(boost::hash_value(proto));
      foreach (const rObject& o, content_)
      {
        res->combine(o);
      }
      return res;
    }

    // FIXME: Really looks like String::join, we should find a means to
    // factor both.
    std::string
    List::as_string() const
    {
      URBI_AT_HOOK(contentChanged);
      std::string res = "[";
      bool first = true;
      foreach (const rObject& o, content_)
      {
        if (!first)
          res += ", ";
        first = false;
        res += o->call(SYMBOL(asPrintable))->as<String>()->value_get();
      }
      res += "]";
      return res;
    }


    rObject
    List::removeBack()
    {
      URBI_AT_HOOK(contentChanged);
      CHECK_NON_EMPTY(pop_back);
      rObject res = content_.back();
      content_.pop_back();
      sizeChanged();
      contentChanged();
      return res;
    }

    rObject
    List::removeFront()
    {
      URBI_AT_HOOK(contentChanged);
      CHECK_NON_EMPTY(pop_front);
      rObject res = content_.front();
      content_.pop_front();
      sizeChanged();
      contentChanged();
      return res;
    }

    rList List::reverse() const
    {
      URBI_AT_HOOK(contentChanged);
      value_type res;
      rforeach (const rObject& obj, content_)
        res.push_back(obj);
      return new List(res);
    }

    /*
       SYMBOL(sizeChanged)
       SYMBOL(contentChanged)
     */
    URBI_ATTRIBUTE_ON_DEMAND_IMPL(List, Event, sizeChanged);
    URBI_ATTRIBUTE_ON_DEMAND_IMPL(List, Event, contentChanged);
  } // namespace object
}

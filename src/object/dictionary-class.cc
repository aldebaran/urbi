/**
 ** \file object/dictionary-class.cc
 ** \brief Creation of the URBI object dictionary.
 */

#include "dictionary-class.hh"
#include "object/symbols.hh"
#include "runner/runner.hh"

namespace object
{
  rObject dictionary_class;

  namespace
  {
    static rObject
    dictionary_class_set(runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT(3);
      FETCH_ARG(0, Dictionary);
      FETCH_ARG(1, String);
      arg0->value_get()[arg1->value_get()] = args[2];
      return arg0;
    }

    static rObject
    dictionary_class_get(runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT(2);
      FETCH_ARG(0, Dictionary);
      FETCH_ARG(1, String);
      Dict& d = arg0->value_get();
      Dict::iterator it = d.find(arg1->value_get());
      if (it != d.end())
        return it->second;
      else
        return nil_class;
    }

    static rObject
    dictionary_class_has(runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT(2);
      FETCH_ARG(0, Dictionary);
      FETCH_ARG(1, String);
      Dict& d = arg0->value_get();
      return Float::fresh(d.find(arg1->value_get()) != d.end());
    }

    static rObject
    dictionary_class_clone(runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT(1);
      return Dictionary::fresh(Dict());
    }

    static rObject
    dictionary_class_clear(runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT(1);
      FETCH_ARG(0, Dictionary);
      arg0->value_get().clear();
      return arg0;
    }

    static rObject
    dictionary_class_keys(runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT(1);
      FETCH_ARG(0, Dictionary);
      std::deque<rObject> res;
      typedef std::pair<libport::Symbol, rObject> Elt;
      foreach (const Elt& elt, arg0->value_get())
        res.push_back(String::fresh(elt.first));
      return List::fresh(res);
    }
  }

  std::ostream& operator << (std::ostream& where,
                             const Dict& what)
  {
    where << &what << std::endl;
    return where;
  }

  void
  dictionary_class_initialize ()
  {
#define DECLARE(Name)                           \
    DECLARE_PRIMITIVE(dictionary, Name)
    DECLARE(clear);
    DECLARE(clone);
    DECLARE(get);
    DECLARE(has);
    DECLARE(keys);
    DECLARE(set);
#undef DECLARE
  }
}

/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/uvalue.cc
 ** \brief Creation of the Urbi object UValue.
 */

# include <urbi/kernel/userver.hh>
# include <kernel/uvalue-cast.hh>

# include <urbi/object/symbols.hh>
# include <object/uvalue.hh>

# include <urbi/object/list.hh>

namespace urbi
{
  namespace object
  {

    UValue::UValue()
      : bypassMode_(false)
    {
      proto_set(proto ? rObject(proto) : CxxObject::proto);
    }

    UValue::UValue(libport::intrusive_ptr<UValue>)
      : bypassMode_(false)
    {
      proto_set(proto ? rObject(proto) : CxxObject::proto);
    }

    UValue::UValue(const urbi::UValue& v, bool bypass)
      : bypassMode_(false)
    {
      proto_set(proto ? rObject(proto) : CxxObject::proto);
      put(v, bypass);
    }

    URBI_CXX_OBJECT_INIT(UValue)
    {
#define DECLARE(Name)                   \
      bind(SYMBOL_(Name), &UValue::Name)

      DECLARE(extract);
      DECLARE(extractAsToplevelPrintable);
      DECLARE(invalidate);

#undef DECLARE

      bind(SYMBOL(put), (void (UValue::*)(rObject))&UValue::put);
    }

    UValue::~UValue()
    {}

    rObject
    UValue::extract()
    {
      if (cache_)
        return cache_;
      if (value_.type == urbi::DATA_VOID)
        return nil_class;
      if (!cache_)
        cache_ = object_cast(value_);
      return cache_;
    }

    std::string
    UValue::extractAsToplevelPrintable()
    {
      if (value_.type == urbi::DATA_VOID && cache_)
      { // Someone used put, cache_ is more recent
        return cache_->call("asToplevelPrintable")->as<String>()->value_get();
      }
      std::stringstream s;
      value_.print(s);
      std::string res = s.str();
      if (value_.type == urbi::DATA_BINARY)
      {
        /** Problem:  the Binary was printed with ';' as a separator, and we
        *  need '\n'. The clean thing would be to convert the UBinary to
        * its rObject version and call asPrintable, but it implies an extra
        * copy.
        */
        size_t pos = res.find_first_of(";");
        if (pos != res.npos)
          res[pos] = '\n';
      }
      return res;
    }

    const urbi::UValue&
    UValue::value_get()
    {
      static urbi::UValue dummy;
      if (value_.type != urbi::DATA_VOID)
        return value_;
      if (!cache_ || cache_ == nil_class)
        return dummy;
      value_ = ::uvalue_cast(cache_);
      allocated_ = true;
      return value_;
    }

    void
    UValue::invalidate()
    {
      if (!allocated_)
        value_ = urbi::UValue();
    }

    void
    UValue::put(const urbi::UValue& v,  bool bypass)
    {
      bypassMode_ = bypass;
      allocated_ = !bypass;
      value_.set(v, !bypass);
      if (value_.type == DATA_BINARY)
        value_.binary->temporary_ = false;
      cache_ = 0;
    }

    void
    UValue::put(rObject r)
    {
      value_ = urbi::UValue();
      cache_ = r;
    }
  }
}

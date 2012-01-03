/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_STRING_UVALUE_HH
# define OBJECT_STRING_UVALUE_HH

# include <runner/runner.hh>
# include <urbi/object/cxx-object.hh>
# include <urbi/uvalue.hh>

namespace urbi
{
  namespace object
  {

    /** K2-object representation of a UValue.
     * May contain a rObject, an urbi::UValue, or both: the one that mirrors the
     * other is created on first request and kept.
     * The urbi::UValue may not be copied if put is called with bypass=false. In
     * that case, invalidate() must be called to notify the UValue that its
     * urbi::UValue is no longer valid.
     */
    class UValue: public CxxObject
    {
    public:
      UValue();
      UValue(libport::intrusive_ptr<UValue> model);
      // Keep a reference to v.
      UValue(const urbi::UValue& v, bool bypass=false);
      UValue(rObject v);
      ~UValue();
      void put(const urbi::UValue& v, bool bypass=false);
      void put(rObject v);
      rObject extract();
      std::string extractAsToplevelPrintable();
      void invalidate();
      const urbi::UValue& value_get();
    private:
      urbi::UValue value_;
      /// False if the value was constructed with copy=false.
      bool allocated_;
      rObject cache_;
      bool bypassMode_;
      friend class ::urbi::object::UVar;
      URBI_CXX_OBJECT(UValue, CxxObject);
    };

  }
}
#endif // !OBJECT_STRING_UVALUE_HH

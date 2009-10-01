/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#ifndef OBJECT_STRING_UVAR_HH
# define OBJECT_STRING_UVAR_HH

# include <object/cxx-object.hh>
# include <runner/runner.hh>

# include <urbi/uvalue.hh>
namespace object
{
  /** class providing access and change notification of a slot.
   *
   *  See uobject.u for the urbi part.
   */
  class UVar: public Primitive
  {
  public:
    UVar();
    UVar(libport::intrusive_ptr<UVar> model);
    rObject update_(rObject arg);
    rObject accessor();
    /// Like accessor, but if fromCxx is true and the value is an UValue,
    /// return it instead of its content.
    rObject getter(bool fromCXX);
    void loopCheck();
    rObject writeOwned(rObject newval);
  private:
    bool looping_;
    /// Set of runners currently in a notifyChange.
    std::set<void*> inChange_;
    bool inAccess_;
    URBI_CXX_OBJECT(UVar);
  };
  typedef libport::intrusive_ptr<UVar> rUVar;

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
    bool alocated_;
    rObject cache_;
    URBI_CXX_OBJECT(UValue);
  };
  typedef libport::intrusive_ptr<UValue> rUValue;
}
#endif

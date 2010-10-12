/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/uvar.hh
#ifndef URBI_CUSTOMUVAR_HH
# define URBI_CUSTOMUVAR_HH

#include <urbi/uobject.hh>
#include <urbi/uvar.hh>

namespace urbi
{
  /** UVar wrapper with a custom data field.
   * Since the UVar reference passed as argument to the notifyChange callback
   * is the same as the one given to UNotifyChange, one can inherit from UVar
   * to transmit custom data to the callbacks. This class is designed to ease
   * this process.
   *
   * Example usage:
   *
   * \code
   * CustomUVar<int> v = new UVar(*this, "motor" + string_cast(mID));
   * v->data() = mID;
   * UNotifyChange(*v, &Motor::onChange);
   *
   * void Motor::onChange(UVar& var)
   * {
   *    int id = CustomUVar<int>::data(var);
   *    writeMotor(id, (double)var);
   * }
   *
   * \endcode
   */
  template<typename T> class CustomUVar: public UVar
  {
  public:
    CustomUVar(const std::string&, impl::UContextImpl* = 0);
    CustomUVar(const std::string&, const std::string&, impl::UContextImpl* = 0);
    CustomUVar(UObject&, const std::string&, impl::UContextImpl* = 0);
    T& data();
    /** @return a reference to the data structure from an UVar.
     * The UVar must be a CustomUVar<T>, or the behavior will be undefined.
     */
    static T& data(UVar&);
  private:
    T data_;
  };
}

# include <urbi/customuvar.hxx>

#endif

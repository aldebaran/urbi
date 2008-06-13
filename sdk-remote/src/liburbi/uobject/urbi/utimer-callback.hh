/*! \file urbi/ucallbacks.hh
 *******************************************************************************

 Definition of the callback classes.

 This file is part of UObject Component Architecture\n
 Copyright (c) 2006, 2007, 2008 Gostai S.A.S.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#ifndef URBI_UTIMER_CALLBACK_HH
# define URBI_UTIMER_CALLBACK_HH

# include <string>
# include <urbi/utimer-table.hh>

namespace urbi
{

  //! Timer mechanism
  /*! This class stores a callback as a class method
   */

  class UTimerCallback
  {
  public:
    UTimerCallback(const std::string& objname, ufloat period, UTimerTable& tt);
    virtual ~UTimerCallback();

    virtual void call() = 0;

    ufloat period;
    ufloat lastTimeCalled;
    std::string objname;
  };

  // UTimerCallback subclasses

  template <class T>
  class UTimerCallbackobj : public UTimerCallback
  {
  public:
# define MKUTimerCallBackObj(Const, IsConst)				\
    UTimerCallbackobj(const std::string& objname,			\
		      ufloat period, T* obj,				\
		      int (T::*fun) () Const, UTimerTable &tt)		\
      : UTimerCallback(objname, period,tt), obj(obj),			\
	fun##Const(fun), is_const_ (IsConst)				\
    {}

    MKUTimerCallBackObj (/**/, false);
    MKUTimerCallBackObj (const, true);

# undef MKUTimerCallBackObj

    virtual void call()
    {
      (is_const_) ? ((*obj).*funconst)() : ((*obj).*fun)();
    }
  private:
    T* obj;
    int (T::*fun) ();
    int (T::*funconst) () const;
    bool is_const_;
  };


} // end namespace urbi

#endif // ! URBI_UTIMER_CALLBACK_HH

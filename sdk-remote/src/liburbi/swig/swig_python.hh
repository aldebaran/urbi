/*! \file swig_python.hh
 * Python binding.
 */
#ifndef SWIG_PYTHON_H
# define SWIG_PYTHON_H

# ifndef SWIGPYTHON
#  error "This file must be compiled with SWIG in python mode."
# endif /* !SWIGPYTHON */

# include <object.h>
# include "uclient.h"

namespace urbi
{
  /*!
   * Pseudo-callback to bind a C callback
   * to a python function.
   */
  class Callback : public UCallbackWrapper
    {
    public:
      Callback(PyObject* obj) :
	UCallbackWrapper(),
	obj_ (obj)
	{
	  if (!obj_ || !PyFunction_Check(obj_) || !PyCallable_Check(obj_))
	    {
	      PyErr_SetString (PyExc_TypeError, "Need a callable object!");
	      return;
	    }
	  Py_INCREF(obj_);
	}

      virtual UCallbackAction operator () (const UMessage& msg)
	{
 	  FILE *fp = fopen("/tmp/foo.txt","w");
 	  PyObject_Print(obj_,fp,Py_PRINT_RAW);
 	  fclose(fp);

	  PyObject_CallFunction(obj_,"s", msg.message.c_str());
	  return URBI_CONTINUE;
	}

      virtual ~Callback()
	{
	  Py_DECREF(obj_);
	}

    private:
      PyObject* obj_;
    };
};

#endif /* !SWIG_PYTHON_H */

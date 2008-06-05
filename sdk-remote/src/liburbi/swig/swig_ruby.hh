/*! \file swig_ruby.hh
 * Ruby binding.
 */
#ifndef SWIG_RUBY_H
# define SWIG_RUBY_H

# ifndef SWIGRUBY
#  error "This file must be compiled with SWIG in ruby mode."
# endif /* !SWIGRUBY */

# include <uclient.h>

namespace urbi
{
 class Callback : public UCallbackWrapper
   {
   public:
     Callback(VALUE proc) :
       UCallbackWrapper(),
       proc_ (proc)
       {
	 Check_Type(proc_, T_DATA);
       }

     virtual UCallbackAction operator () (const UMessage& msg)
       {
	 // v est le paramètre que je vais passer au callback
	 // je veux qu'ils contiennent les mêmes informations que msg.
	 VALUE v = Qnil;
	 v = SWIG_Ruby_NewPointerObj((void *)&msg,
				     SWIGTYPE_p_urbi__UMessage,
				     0);

	 rb_funcall(proc_, rb_intern("call"), 1, v);

	 return URBI_CONTINUE;
       }

     virtual ~Callback() {}

   private:
     VALUE proc_;
   };
};

#endif /* !SWIG_RUBY_H */

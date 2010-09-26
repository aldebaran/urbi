/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/* This file was generated with ./generate_call_macros.sh, do not edit directly !*/

#ifndef CALL_MACROS_HH_
# define CALL_MACROS_HH_

# define CALL_METHOD_0(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_0 ()				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          					\
          jvalue argument[] = {  };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_1(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_1 (urbi::UValue uval1)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1);					\
          jvalue argument[] = { obj1 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_2(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_2 (urbi::UValue uval1, urbi::UValue uval2)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1); const jvalue obj2 = getObjectFrom (arg_types[1], uval2);					\
          jvalue argument[] = { obj1, obj2 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_3(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_3 (urbi::UValue uval1, urbi::UValue uval2, urbi::UValue uval3)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1); const jvalue obj2 = getObjectFrom (arg_types[1], uval2); const jvalue obj3 = getObjectFrom (arg_types[2], uval3);					\
          jvalue argument[] = { obj1, obj2, obj3 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_4(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_4 (urbi::UValue uval1, urbi::UValue uval2, urbi::UValue uval3, urbi::UValue uval4)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1); const jvalue obj2 = getObjectFrom (arg_types[1], uval2); const jvalue obj3 = getObjectFrom (arg_types[2], uval3); const jvalue obj4 = getObjectFrom (arg_types[3], uval4);					\
          jvalue argument[] = { obj1, obj2, obj3, obj4 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_5(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_5 (urbi::UValue uval1, urbi::UValue uval2, urbi::UValue uval3, urbi::UValue uval4, urbi::UValue uval5)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1); const jvalue obj2 = getObjectFrom (arg_types[1], uval2); const jvalue obj3 = getObjectFrom (arg_types[2], uval3); const jvalue obj4 = getObjectFrom (arg_types[3], uval4); const jvalue obj5 = getObjectFrom (arg_types[4], uval5);					\
          jvalue argument[] = { obj1, obj2, obj3, obj4, obj5 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_6(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_6 (urbi::UValue uval1, urbi::UValue uval2, urbi::UValue uval3, urbi::UValue uval4, urbi::UValue uval5, urbi::UValue uval6)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1); const jvalue obj2 = getObjectFrom (arg_types[1], uval2); const jvalue obj3 = getObjectFrom (arg_types[2], uval3); const jvalue obj4 = getObjectFrom (arg_types[3], uval4); const jvalue obj5 = getObjectFrom (arg_types[4], uval5); const jvalue obj6 = getObjectFrom (arg_types[5], uval6);					\
          jvalue argument[] = { obj1, obj2, obj3, obj4, obj5, obj6 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_7(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_7 (urbi::UValue uval1, urbi::UValue uval2, urbi::UValue uval3, urbi::UValue uval4, urbi::UValue uval5, urbi::UValue uval6, urbi::UValue uval7)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1); const jvalue obj2 = getObjectFrom (arg_types[1], uval2); const jvalue obj3 = getObjectFrom (arg_types[2], uval3); const jvalue obj4 = getObjectFrom (arg_types[3], uval4); const jvalue obj5 = getObjectFrom (arg_types[4], uval5); const jvalue obj6 = getObjectFrom (arg_types[5], uval6); const jvalue obj7 = getObjectFrom (arg_types[6], uval7);					\
          jvalue argument[] = { obj1, obj2, obj3, obj4, obj5, obj6, obj7 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_8(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_8 (urbi::UValue uval1, urbi::UValue uval2, urbi::UValue uval3, urbi::UValue uval4, urbi::UValue uval5, urbi::UValue uval6, urbi::UValue uval7, urbi::UValue uval8)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1); const jvalue obj2 = getObjectFrom (arg_types[1], uval2); const jvalue obj3 = getObjectFrom (arg_types[2], uval3); const jvalue obj4 = getObjectFrom (arg_types[3], uval4); const jvalue obj5 = getObjectFrom (arg_types[4], uval5); const jvalue obj6 = getObjectFrom (arg_types[5], uval6); const jvalue obj7 = getObjectFrom (arg_types[6], uval7); const jvalue obj8 = getObjectFrom (arg_types[7], uval8);					\
          jvalue argument[] = { obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_9(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_9 (urbi::UValue uval1, urbi::UValue uval2, urbi::UValue uval3, urbi::UValue uval4, urbi::UValue uval5, urbi::UValue uval6, urbi::UValue uval7, urbi::UValue uval8, urbi::UValue uval9)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1); const jvalue obj2 = getObjectFrom (arg_types[1], uval2); const jvalue obj3 = getObjectFrom (arg_types[2], uval3); const jvalue obj4 = getObjectFrom (arg_types[3], uval4); const jvalue obj5 = getObjectFrom (arg_types[4], uval5); const jvalue obj6 = getObjectFrom (arg_types[5], uval6); const jvalue obj7 = getObjectFrom (arg_types[6], uval7); const jvalue obj8 = getObjectFrom (arg_types[7], uval8); const jvalue obj9 = getObjectFrom (arg_types[8], uval9);					\
          jvalue argument[] = { obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_10(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_10 (urbi::UValue uval1, urbi::UValue uval2, urbi::UValue uval3, urbi::UValue uval4, urbi::UValue uval5, urbi::UValue uval6, urbi::UValue uval7, urbi::UValue uval8, urbi::UValue uval9, urbi::UValue uval10)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1); const jvalue obj2 = getObjectFrom (arg_types[1], uval2); const jvalue obj3 = getObjectFrom (arg_types[2], uval3); const jvalue obj4 = getObjectFrom (arg_types[3], uval4); const jvalue obj5 = getObjectFrom (arg_types[4], uval5); const jvalue obj6 = getObjectFrom (arg_types[5], uval6); const jvalue obj7 = getObjectFrom (arg_types[6], uval7); const jvalue obj8 = getObjectFrom (arg_types[7], uval8); const jvalue obj9 = getObjectFrom (arg_types[8], uval9); const jvalue obj10 = getObjectFrom (arg_types[9], uval10);					\
          jvalue argument[] = { obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_11(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_11 (urbi::UValue uval1, urbi::UValue uval2, urbi::UValue uval3, urbi::UValue uval4, urbi::UValue uval5, urbi::UValue uval6, urbi::UValue uval7, urbi::UValue uval8, urbi::UValue uval9, urbi::UValue uval10, urbi::UValue uval11)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1); const jvalue obj2 = getObjectFrom (arg_types[1], uval2); const jvalue obj3 = getObjectFrom (arg_types[2], uval3); const jvalue obj4 = getObjectFrom (arg_types[3], uval4); const jvalue obj5 = getObjectFrom (arg_types[4], uval5); const jvalue obj6 = getObjectFrom (arg_types[5], uval6); const jvalue obj7 = getObjectFrom (arg_types[6], uval7); const jvalue obj8 = getObjectFrom (arg_types[7], uval8); const jvalue obj9 = getObjectFrom (arg_types[8], uval9); const jvalue obj10 = getObjectFrom (arg_types[9], uval10); const jvalue obj11 = getObjectFrom (arg_types[10], uval11);					\
          jvalue argument[] = { obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10, obj11 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_12(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_12 (urbi::UValue uval1, urbi::UValue uval2, urbi::UValue uval3, urbi::UValue uval4, urbi::UValue uval5, urbi::UValue uval6, urbi::UValue uval7, urbi::UValue uval8, urbi::UValue uval9, urbi::UValue uval10, urbi::UValue uval11, urbi::UValue uval12)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1); const jvalue obj2 = getObjectFrom (arg_types[1], uval2); const jvalue obj3 = getObjectFrom (arg_types[2], uval3); const jvalue obj4 = getObjectFrom (arg_types[3], uval4); const jvalue obj5 = getObjectFrom (arg_types[4], uval5); const jvalue obj6 = getObjectFrom (arg_types[5], uval6); const jvalue obj7 = getObjectFrom (arg_types[6], uval7); const jvalue obj8 = getObjectFrom (arg_types[7], uval8); const jvalue obj9 = getObjectFrom (arg_types[8], uval9); const jvalue obj10 = getObjectFrom (arg_types[9], uval10); const jvalue obj11 = getObjectFrom (arg_types[10], uval11); const jvalue obj12 = getObjectFrom (arg_types[11], uval12);					\
          jvalue argument[] = { obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10, obj11, obj12 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_13(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_13 (urbi::UValue uval1, urbi::UValue uval2, urbi::UValue uval3, urbi::UValue uval4, urbi::UValue uval5, urbi::UValue uval6, urbi::UValue uval7, urbi::UValue uval8, urbi::UValue uval9, urbi::UValue uval10, urbi::UValue uval11, urbi::UValue uval12, urbi::UValue uval13)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1); const jvalue obj2 = getObjectFrom (arg_types[1], uval2); const jvalue obj3 = getObjectFrom (arg_types[2], uval3); const jvalue obj4 = getObjectFrom (arg_types[3], uval4); const jvalue obj5 = getObjectFrom (arg_types[4], uval5); const jvalue obj6 = getObjectFrom (arg_types[5], uval6); const jvalue obj7 = getObjectFrom (arg_types[6], uval7); const jvalue obj8 = getObjectFrom (arg_types[7], uval8); const jvalue obj9 = getObjectFrom (arg_types[8], uval9); const jvalue obj10 = getObjectFrom (arg_types[9], uval10); const jvalue obj11 = getObjectFrom (arg_types[10], uval11); const jvalue obj12 = getObjectFrom (arg_types[11], uval12); const jvalue obj13 = getObjectFrom (arg_types[12], uval13);					\
          jvalue argument[] = { obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10, obj11, obj12, obj13 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_14(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_14 (urbi::UValue uval1, urbi::UValue uval2, urbi::UValue uval3, urbi::UValue uval4, urbi::UValue uval5, urbi::UValue uval6, urbi::UValue uval7, urbi::UValue uval8, urbi::UValue uval9, urbi::UValue uval10, urbi::UValue uval11, urbi::UValue uval12, urbi::UValue uval13, urbi::UValue uval14)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1); const jvalue obj2 = getObjectFrom (arg_types[1], uval2); const jvalue obj3 = getObjectFrom (arg_types[2], uval3); const jvalue obj4 = getObjectFrom (arg_types[3], uval4); const jvalue obj5 = getObjectFrom (arg_types[4], uval5); const jvalue obj6 = getObjectFrom (arg_types[5], uval6); const jvalue obj7 = getObjectFrom (arg_types[6], uval7); const jvalue obj8 = getObjectFrom (arg_types[7], uval8); const jvalue obj9 = getObjectFrom (arg_types[8], uval9); const jvalue obj10 = getObjectFrom (arg_types[9], uval10); const jvalue obj11 = getObjectFrom (arg_types[10], uval11); const jvalue obj12 = getObjectFrom (arg_types[11], uval12); const jvalue obj13 = getObjectFrom (arg_types[12], uval13); const jvalue obj14 = getObjectFrom (arg_types[13], uval14);					\
          jvalue argument[] = { obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10, obj11, obj12, obj13, obj14 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_15(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_15 (urbi::UValue uval1, urbi::UValue uval2, urbi::UValue uval3, urbi::UValue uval4, urbi::UValue uval5, urbi::UValue uval6, urbi::UValue uval7, urbi::UValue uval8, urbi::UValue uval9, urbi::UValue uval10, urbi::UValue uval11, urbi::UValue uval12, urbi::UValue uval13, urbi::UValue uval14, urbi::UValue uval15)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1); const jvalue obj2 = getObjectFrom (arg_types[1], uval2); const jvalue obj3 = getObjectFrom (arg_types[2], uval3); const jvalue obj4 = getObjectFrom (arg_types[3], uval4); const jvalue obj5 = getObjectFrom (arg_types[4], uval5); const jvalue obj6 = getObjectFrom (arg_types[5], uval6); const jvalue obj7 = getObjectFrom (arg_types[6], uval7); const jvalue obj8 = getObjectFrom (arg_types[7], uval8); const jvalue obj9 = getObjectFrom (arg_types[8], uval9); const jvalue obj10 = getObjectFrom (arg_types[9], uval10); const jvalue obj11 = getObjectFrom (arg_types[10], uval11); const jvalue obj12 = getObjectFrom (arg_types[11], uval12); const jvalue obj13 = getObjectFrom (arg_types[12], uval13); const jvalue obj14 = getObjectFrom (arg_types[13], uval14); const jvalue obj15 = getObjectFrom (arg_types[14], uval15);					\
          jvalue argument[] = { obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10, obj11, obj12, obj13, obj14, obj15 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHOD_16(Name, Type, JavaType, error_val, ret, ret_snd)	\
	JavaType call##Name##_16 (urbi::UValue uval1, urbi::UValue uval2, urbi::UValue uval3, urbi::UValue uval4, urbi::UValue uval5, urbi::UValue uval6, urbi::UValue uval7, urbi::UValue uval8, urbi::UValue uval9, urbi::UValue uval10, urbi::UValue uval11, urbi::UValue uval12, urbi::UValue uval13, urbi::UValue uval14, urbi::UValue uval15, urbi::UValue uval16)				\
	{								\
	  if (!init_env ())						\
	    return error_val;						\
          const jvalue obj1 = getObjectFrom (arg_types[0], uval1); const jvalue obj2 = getObjectFrom (arg_types[1], uval2); const jvalue obj3 = getObjectFrom (arg_types[2], uval3); const jvalue obj4 = getObjectFrom (arg_types[3], uval4); const jvalue obj5 = getObjectFrom (arg_types[4], uval5); const jvalue obj6 = getObjectFrom (arg_types[5], uval6); const jvalue obj7 = getObjectFrom (arg_types[6], uval7); const jvalue obj8 = getObjectFrom (arg_types[7], uval8); const jvalue obj9 = getObjectFrom (arg_types[8], uval9); const jvalue obj10 = getObjectFrom (arg_types[9], uval10); const jvalue obj11 = getObjectFrom (arg_types[10], uval11); const jvalue obj12 = getObjectFrom (arg_types[11], uval12); const jvalue obj13 = getObjectFrom (arg_types[12], uval13); const jvalue obj14 = getObjectFrom (arg_types[13], uval14); const jvalue obj15 = getObjectFrom (arg_types[14], uval15); const jvalue obj16 = getObjectFrom (arg_types[15], uval16);					\
          jvalue argument[] = { obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10, obj11, obj12, obj13, obj14, obj15, obj16 };                                                     \
	  ret env_->Call##Type##MethodA(obj, mid, argument);   		\
          testForException();						\
          ret_snd;							\
	}
# define CALL_METHODS(Name, Type, JavaType, error_val, ret, ret_snd)		\
  CALL_METHOD_0 (Name, Type, JavaType, error_val, ret, ret_snd); CALL_METHOD_1 (Name, Type, JavaType, error_val, ret, ret_snd); CALL_METHOD_2 (Name, Type, JavaType, error_val, ret, ret_snd); CALL_METHOD_3 (Name, Type, JavaType, error_val, ret, ret_snd); CALL_METHOD_4 (Name, Type, JavaType, error_val, ret, ret_snd); CALL_METHOD_5 (Name, Type, JavaType, error_val, ret, ret_snd); CALL_METHOD_6 (Name, Type, JavaType, error_val, ret, ret_snd); CALL_METHOD_7 (Name, Type, JavaType, error_val, ret, ret_snd); CALL_METHOD_8 (Name, Type, JavaType, error_val, ret, ret_snd); CALL_METHOD_9 (Name, Type, JavaType, error_val, ret, ret_snd); CALL_METHOD_10 (Name, Type, JavaType, error_val, ret, ret_snd); CALL_METHOD_11 (Name, Type, JavaType, error_val, ret, ret_snd); CALL_METHOD_12 (Name, Type, JavaType, error_val, ret, ret_snd); CALL_METHOD_13 (Name, Type, JavaType, error_val, ret, ret_snd); CALL_METHOD_14 (Name, Type, JavaType, error_val, ret, ret_snd); CALL_METHOD_15 (Name, Type, JavaType, error_val, ret, ret_snd);								\
  CALL_METHOD_16 (Name, Type, JavaType, error_val, ret, ret_snd);



#endif

/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef CALL_MACROS_HH_
# define CALL_MACROS_HH_

# define CALL_METHOD_0(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_0 ()		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
	  ret env_->Call##Type##Method(obj, mid);		\
          ret_snd;						\
	}

# define CALL_METHOD_1(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_1 (const urbi::UValue& uval1)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
	  ret env_->Call##Type##Method(obj, mid, obj1);		\
          ret_snd;						\
	}

# define CALL_METHOD_2(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_2 (const urbi::UValue& uval1, const urbi::UValue& uval2)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
          jobject obj2 = getObjectFromUValue (uval2);		\
	  ret env_->Call##Type##Method(obj, mid, obj1, obj2);		\
          ret_snd;						\
	}

# define CALL_METHOD_3(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_3 (const urbi::UValue& uval1, const urbi::UValue& uval2, const urbi::UValue& uval3)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
          jobject obj2 = getObjectFromUValue (uval2);		\
          jobject obj3 = getObjectFromUValue (uval3);		\
	  ret env_->Call##Type##Method(obj, mid, obj1, obj2, obj3);		\
          ret_snd;						\
	}

# define CALL_METHOD_4(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_4 (const urbi::UValue& uval1, const urbi::UValue& uval2, const urbi::UValue& uval3, const urbi::UValue& uval4)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
          jobject obj2 = getObjectFromUValue (uval2);		\
          jobject obj3 = getObjectFromUValue (uval3);		\
          jobject obj4 = getObjectFromUValue (uval4);		\
	  ret env_->Call##Type##Method(obj, mid, obj1, obj2, obj3, obj4);		\
          ret_snd;						\
	}

# define CALL_METHOD_5(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_5 (const urbi::UValue& uval1, const urbi::UValue& uval2, const urbi::UValue& uval3, const urbi::UValue& uval4, const urbi::UValue& uval5)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
          jobject obj2 = getObjectFromUValue (uval2);		\
          jobject obj3 = getObjectFromUValue (uval3);		\
          jobject obj4 = getObjectFromUValue (uval4);		\
          jobject obj5 = getObjectFromUValue (uval5);		\
	  ret env_->Call##Type##Method(obj, mid, obj1, obj2, obj3, obj4, obj5);		\
          ret_snd;						\
	}

# define CALL_METHOD_6(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_6 (const urbi::UValue& uval1, const urbi::UValue& uval2, const urbi::UValue& uval3, const urbi::UValue& uval4, const urbi::UValue& uval5, const urbi::UValue& uval6)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
          jobject obj2 = getObjectFromUValue (uval2);		\
          jobject obj3 = getObjectFromUValue (uval3);		\
          jobject obj4 = getObjectFromUValue (uval4);		\
          jobject obj5 = getObjectFromUValue (uval5);		\
          jobject obj6 = getObjectFromUValue (uval6);		\
	  ret env_->Call##Type##Method(obj, mid, obj1, obj2, obj3, obj4, obj5, obj6);		\
          ret_snd;						\
	}

# define CALL_METHOD_7(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_7 (const urbi::UValue& uval1, const urbi::UValue& uval2, const urbi::UValue& uval3, const urbi::UValue& uval4, const urbi::UValue& uval5, const urbi::UValue& uval6, const urbi::UValue& uval7)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
          jobject obj2 = getObjectFromUValue (uval2);		\
          jobject obj3 = getObjectFromUValue (uval3);		\
          jobject obj4 = getObjectFromUValue (uval4);		\
          jobject obj5 = getObjectFromUValue (uval5);		\
          jobject obj6 = getObjectFromUValue (uval6);		\
          jobject obj7 = getObjectFromUValue (uval7);		\
	  ret env_->Call##Type##Method(obj, mid, obj1, obj2, obj3, obj4, obj5, obj6, obj7);		\
          ret_snd;						\
	}

# define CALL_METHOD_8(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_8 (const urbi::UValue& uval1, const urbi::UValue& uval2, const urbi::UValue& uval3, const urbi::UValue& uval4, const urbi::UValue& uval5, const urbi::UValue& uval6, const urbi::UValue& uval7, const urbi::UValue& uval8)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
          jobject obj2 = getObjectFromUValue (uval2);		\
          jobject obj3 = getObjectFromUValue (uval3);		\
          jobject obj4 = getObjectFromUValue (uval4);		\
          jobject obj5 = getObjectFromUValue (uval5);		\
          jobject obj6 = getObjectFromUValue (uval6);		\
          jobject obj7 = getObjectFromUValue (uval7);		\
          jobject obj8 = getObjectFromUValue (uval8);		\
	  ret env_->Call##Type##Method(obj, mid, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);		\
          ret_snd;						\
	}

# define CALL_METHOD_9(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_9 (const urbi::UValue& uval1, const urbi::UValue& uval2, const urbi::UValue& uval3, const urbi::UValue& uval4, const urbi::UValue& uval5, const urbi::UValue& uval6, const urbi::UValue& uval7, const urbi::UValue& uval8, const urbi::UValue& uval9)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
          jobject obj2 = getObjectFromUValue (uval2);		\
          jobject obj3 = getObjectFromUValue (uval3);		\
          jobject obj4 = getObjectFromUValue (uval4);		\
          jobject obj5 = getObjectFromUValue (uval5);		\
          jobject obj6 = getObjectFromUValue (uval6);		\
          jobject obj7 = getObjectFromUValue (uval7);		\
          jobject obj8 = getObjectFromUValue (uval8);		\
          jobject obj9 = getObjectFromUValue (uval9);		\
	  ret env_->Call##Type##Method(obj, mid, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9);		\
          ret_snd;						\
	}

# define CALL_METHOD_10(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_10 (const urbi::UValue& uval1, const urbi::UValue& uval2, const urbi::UValue& uval3, const urbi::UValue& uval4, const urbi::UValue& uval5, const urbi::UValue& uval6, const urbi::UValue& uval7, const urbi::UValue& uval8, const urbi::UValue& uval9, const urbi::UValue& uval10)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
          jobject obj2 = getObjectFromUValue (uval2);		\
          jobject obj3 = getObjectFromUValue (uval3);		\
          jobject obj4 = getObjectFromUValue (uval4);		\
          jobject obj5 = getObjectFromUValue (uval5);		\
          jobject obj6 = getObjectFromUValue (uval6);		\
          jobject obj7 = getObjectFromUValue (uval7);		\
          jobject obj8 = getObjectFromUValue (uval8);		\
          jobject obj9 = getObjectFromUValue (uval9);		\
          jobject obj10 = getObjectFromUValue (uval10);		\
	  ret env_->Call##Type##Method(obj, mid, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10);		\
          ret_snd;						\
	}

# define CALL_METHOD_11(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_11 (const urbi::UValue& uval1, const urbi::UValue& uval2, const urbi::UValue& uval3, const urbi::UValue& uval4, const urbi::UValue& uval5, const urbi::UValue& uval6, const urbi::UValue& uval7, const urbi::UValue& uval8, const urbi::UValue& uval9, const urbi::UValue& uval10, const urbi::UValue& uval11)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
          jobject obj2 = getObjectFromUValue (uval2);		\
          jobject obj3 = getObjectFromUValue (uval3);		\
          jobject obj4 = getObjectFromUValue (uval4);		\
          jobject obj5 = getObjectFromUValue (uval5);		\
          jobject obj6 = getObjectFromUValue (uval6);		\
          jobject obj7 = getObjectFromUValue (uval7);		\
          jobject obj8 = getObjectFromUValue (uval8);		\
          jobject obj9 = getObjectFromUValue (uval9);		\
          jobject obj10 = getObjectFromUValue (uval10);		\
          jobject obj11 = getObjectFromUValue (uval11);		\
	  ret env_->Call##Type##Method(obj, mid, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10, obj11);		\
          ret_snd;						\
	}

# define CALL_METHOD_12(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_12 (const urbi::UValue& uval1, const urbi::UValue& uval2, const urbi::UValue& uval3, const urbi::UValue& uval4, const urbi::UValue& uval5, const urbi::UValue& uval6, const urbi::UValue& uval7, const urbi::UValue& uval8, const urbi::UValue& uval9, const urbi::UValue& uval10, const urbi::UValue& uval11, const urbi::UValue& uval12)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
          jobject obj2 = getObjectFromUValue (uval2);		\
          jobject obj3 = getObjectFromUValue (uval3);		\
          jobject obj4 = getObjectFromUValue (uval4);		\
          jobject obj5 = getObjectFromUValue (uval5);		\
          jobject obj6 = getObjectFromUValue (uval6);		\
          jobject obj7 = getObjectFromUValue (uval7);		\
          jobject obj8 = getObjectFromUValue (uval8);		\
          jobject obj9 = getObjectFromUValue (uval9);		\
          jobject obj10 = getObjectFromUValue (uval10);		\
          jobject obj11 = getObjectFromUValue (uval11);		\
          jobject obj12 = getObjectFromUValue (uval12);		\
	  ret env_->Call##Type##Method(obj, mid, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10, obj11, obj12);		\
          ret_snd;						\
	}

# define CALL_METHOD_13(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_13 (const urbi::UValue& uval1, const urbi::UValue& uval2, const urbi::UValue& uval3, const urbi::UValue& uval4, const urbi::UValue& uval5, const urbi::UValue& uval6, const urbi::UValue& uval7, const urbi::UValue& uval8, const urbi::UValue& uval9, const urbi::UValue& uval10, const urbi::UValue& uval11, const urbi::UValue& uval12, const urbi::UValue& uval13)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
          jobject obj2 = getObjectFromUValue (uval2);		\
          jobject obj3 = getObjectFromUValue (uval3);		\
          jobject obj4 = getObjectFromUValue (uval4);		\
          jobject obj5 = getObjectFromUValue (uval5);		\
          jobject obj6 = getObjectFromUValue (uval6);		\
          jobject obj7 = getObjectFromUValue (uval7);		\
          jobject obj8 = getObjectFromUValue (uval8);		\
          jobject obj9 = getObjectFromUValue (uval9);		\
          jobject obj10 = getObjectFromUValue (uval10);		\
          jobject obj11 = getObjectFromUValue (uval11);		\
          jobject obj12 = getObjectFromUValue (uval12);		\
          jobject obj13 = getObjectFromUValue (uval13);		\
	  ret env_->Call##Type##Method(obj, mid, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10, obj11, obj12, obj13);		\
          ret_snd;						\
	}

# define CALL_METHOD_14(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_14 (const urbi::UValue& uval1, const urbi::UValue& uval2, const urbi::UValue& uval3, const urbi::UValue& uval4, const urbi::UValue& uval5, const urbi::UValue& uval6, const urbi::UValue& uval7, const urbi::UValue& uval8, const urbi::UValue& uval9, const urbi::UValue& uval10, const urbi::UValue& uval11, const urbi::UValue& uval12, const urbi::UValue& uval13, const urbi::UValue& uval14)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
          jobject obj2 = getObjectFromUValue (uval2);		\
          jobject obj3 = getObjectFromUValue (uval3);		\
          jobject obj4 = getObjectFromUValue (uval4);		\
          jobject obj5 = getObjectFromUValue (uval5);		\
          jobject obj6 = getObjectFromUValue (uval6);		\
          jobject obj7 = getObjectFromUValue (uval7);		\
          jobject obj8 = getObjectFromUValue (uval8);		\
          jobject obj9 = getObjectFromUValue (uval9);		\
          jobject obj10 = getObjectFromUValue (uval10);		\
          jobject obj11 = getObjectFromUValue (uval11);		\
          jobject obj12 = getObjectFromUValue (uval12);		\
          jobject obj13 = getObjectFromUValue (uval13);		\
          jobject obj14 = getObjectFromUValue (uval14);		\
	  ret env_->Call##Type##Method(obj, mid, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10, obj11, obj12, obj13, obj14);		\
          ret_snd;						\
	}

# define CALL_METHOD_15(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_15 (const urbi::UValue& uval1, const urbi::UValue& uval2, const urbi::UValue& uval3, const urbi::UValue& uval4, const urbi::UValue& uval5, const urbi::UValue& uval6, const urbi::UValue& uval7, const urbi::UValue& uval8, const urbi::UValue& uval9, const urbi::UValue& uval10, const urbi::UValue& uval11, const urbi::UValue& uval12, const urbi::UValue& uval13, const urbi::UValue& uval14, const urbi::UValue& uval15)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
          jobject obj2 = getObjectFromUValue (uval2);		\
          jobject obj3 = getObjectFromUValue (uval3);		\
          jobject obj4 = getObjectFromUValue (uval4);		\
          jobject obj5 = getObjectFromUValue (uval5);		\
          jobject obj6 = getObjectFromUValue (uval6);		\
          jobject obj7 = getObjectFromUValue (uval7);		\
          jobject obj8 = getObjectFromUValue (uval8);		\
          jobject obj9 = getObjectFromUValue (uval9);		\
          jobject obj10 = getObjectFromUValue (uval10);		\
          jobject obj11 = getObjectFromUValue (uval11);		\
          jobject obj12 = getObjectFromUValue (uval12);		\
          jobject obj13 = getObjectFromUValue (uval13);		\
          jobject obj14 = getObjectFromUValue (uval14);		\
          jobject obj15 = getObjectFromUValue (uval15);		\
	  ret env_->Call##Type##Method(obj, mid, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10, obj11, obj12, obj13, obj14, obj15);		\
          ret_snd;						\
	}

# define CALL_METHOD_16(Name, Type, JavaType, error_val, ret, ret_snd)\
	JavaType call##Name##_16 (const urbi::UValue& uval1, const urbi::UValue& uval2, const urbi::UValue& uval3, const urbi::UValue& uval4, const urbi::UValue& uval5, const urbi::UValue& uval6, const urbi::UValue& uval7, const urbi::UValue& uval8, const urbi::UValue& uval9, const urbi::UValue& uval10, const urbi::UValue& uval11, const urbi::UValue& uval12, const urbi::UValue& uval13, const urbi::UValue& uval14, const urbi::UValue& uval15, const urbi::UValue& uval16)		\
	{							\
	  if (!env_ && !init_env ())				       	\
	    return error_val;					\
          jobject obj1 = getObjectFromUValue (uval1);		\
          jobject obj2 = getObjectFromUValue (uval2);		\
          jobject obj3 = getObjectFromUValue (uval3);		\
          jobject obj4 = getObjectFromUValue (uval4);		\
          jobject obj5 = getObjectFromUValue (uval5);		\
          jobject obj6 = getObjectFromUValue (uval6);		\
          jobject obj7 = getObjectFromUValue (uval7);		\
          jobject obj8 = getObjectFromUValue (uval8);		\
          jobject obj9 = getObjectFromUValue (uval9);		\
          jobject obj10 = getObjectFromUValue (uval10);		\
          jobject obj11 = getObjectFromUValue (uval11);		\
          jobject obj12 = getObjectFromUValue (uval12);		\
          jobject obj13 = getObjectFromUValue (uval13);		\
          jobject obj14 = getObjectFromUValue (uval14);		\
          jobject obj15 = getObjectFromUValue (uval15);		\
          jobject obj16 = getObjectFromUValue (uval16);		\
	  ret env_->Call##Type##Method(obj, mid, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10, obj11, obj12, obj13, obj14, obj15, obj16);		\
          ret_snd;						\
	}

# define CALL_METHODS(Name, Type, JavaType, error_val, ret, ret_snd)	\
  CALL_METHOD_0 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_1 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_2 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_3 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_4 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_5 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_6 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_7 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_8 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_9 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_10 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_11 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_12 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_13 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_14 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_15 (Name, Type, JavaType, error_val, ret, ret_snd);		\
  CALL_METHOD_16 (Name, Type, JavaType, error_val, ret, ret_snd);

#endif

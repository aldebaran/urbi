/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef CONVERTER_HH_
# define CONVERTER_HH_

# include <jni.h>
# include <urbi/uobject.hh>
# include <urbi/ucallbacks.hh>

# define FRAISE(...)                                            \
  throw std::runtime_error(libport::format(__VA_ARGS__))

class Converter
{
public:
  jvalue convert(JNIEnv* env, const urbi::UValue& val)
  {
    jval = convert_(env, val);
    return jval;
  }

  jvalue convert(JNIEnv* env, urbi::UVar& val)
  {
    jval = convert_(env, val);
    return jval;
  }

  void destroy(JNIEnv* env)
  {
    destroy_(env, jval);
  }

public:
  static Converter* instance(const std::string& type_name,
			     bool is_notify_change_arg = false);

protected:
  virtual void destroy_(JNIEnv* env, jvalue j) {}
  virtual jvalue convert_ (JNIEnv* env, const urbi::UValue& val)
  {
    assert (0);
    return jval;
  };
  virtual jvalue convert_ (JNIEnv* env, urbi::UVar& val)
  {
    return convert_(env, val.val());
  };

  static void allocationCheck(jobject jobj, const std::string& name)
  {
    if (!jobj)
      std::cerr << "Cannot allocate a new object of type "
		<< name << std::endl;
  }

private:
  jvalue jval;
};

class ObjectConverter : public Converter
{
protected:
  void destroy_(JNIEnv* env, jvalue jval)
  {
    if (jval.l)
      env->DeleteLocalRef(jval.l);
  }

  static jclass getGlobalRef (JNIEnv* env, const char* classname)
  {
    /// Get the jclass for UValue
    jclass tmp, res;
    if (!(tmp = env->FindClass(classname)))
      FRAISE("Can't find class %s", classname);
    if (!(res = (jclass) env->NewGlobalRef(tmp)))
      FRAISE("Can't create Global Ref for class %s", classname);
    env->DeleteLocalRef(tmp);
    return res;
  };
};

// Convert simple types that only need casting
# define CAST_CONVERTER(Name, Type, Precast)			\
  class Name##Converter : public Converter			\
  {								\
  protected:							\
    jvalue convert_ (JNIEnv* env, const urbi::UValue& val)	\
    {								\
      const Type& v = (Type)(Precast)val;			\
      return *(jvalue*)&v;					\
    }								\
  };

// Convert object that needs allocation on a java object using NewObject
// and java constructor
# define OBJECT_CONVERTER(Name,						\
                          Type,						\
                          JavaName,					\
		          MethodName,					\
		          MethodArgs)					\
  class Name##Converter : public ObjectConverter			\
  {									\
    typedef ObjectConverter super;					\
									\
  public:								\
    Name##Converter () : allocated(0) {}				\
									\
  protected:								\
    jvalue convert_ (JNIEnv* env, const urbi::UValue& val)		\
    {									\
      jobject jobj = env->NewObject(cls, mid,				\
				    (jlong) alloc(val),			\
				    false);				\
      allocationCheck(jobj, #Name);					\
      return *(jvalue*)&jobj;						\
    }									\
									\
    virtual Type* alloc(const urbi::UValue& val)			\
    {									\
      return (Type*)(allocated = new Type((const Type&)val));		\
    }									\
									\
    virtual void destroy_(JNIEnv* env, jvalue jval)			\
    {									\
      super::destroy_(env, jval);					\
      delete allocated;							\
    }									\
									\
  public:								\
    static void init(JNIEnv* env)					\
    {									\
      if (!(cls = getGlobalRef (env, JavaName)))			\
	FRAISE("Can't find %s class", #Name);                           \
      if (!(mid = env->GetMethodID(cls, MethodName, MethodArgs)))	\
	FRAISE("Can't find %s constructor", #Name);			\
    }									\
  protected:								\
    Type* allocated;							\
									\
  public:								\
    static jclass cls;							\
    static jmethodID mid;						\
  };

# define PRIMITIVE_OBJECT_CONVERTER(Name, Type, Precast, JavaName,	\
                                    MethodName, MethodArgs)		\
  class Name##Converter : public ObjectConverter			\
  {									\
  protected:								\
    jvalue convert_(JNIEnv* env, const urbi::UValue& val)		\
    {									\
      jobject jobj							\
	= env->CallStaticObjectMethod(cls, mid,				\
				      static_cast<Type>((Precast)val));	\
      allocationCheck(jobj, #Name);					\
      return *(jvalue*)&jobj;						\
    }									\
									\
  public:								\
    static void init(JNIEnv* env)					\
    {									\
      if (!(cls = getGlobalRef (env, JavaName)))			\
	FRAISE("Can't find %s class", #Name);                           \
      if (!(mid = env->GetStaticMethodID(cls, MethodName, MethodArgs))) \
	FRAISE("Can't find %s %s function", #Name, MethodName);         \
    }									\
									\
  public:								\
    static jclass cls;							\
    static jmethodID mid;						\
  };

class StringConverter : public ObjectConverter
{
public:
  static jstring staticConvert(JNIEnv* env, const std::string& val)
  {
    jobject jobj = env->NewStringUTF(val.c_str());
    allocationCheck(jobj, "String");
    return (jstring) jobj;
  }

protected:
  jvalue convert_ (JNIEnv* env, const urbi::UValue& val)
  {
    jobject jobj = staticConvert(env, val);
    return *(jvalue*)&jobj;
  }
};

CAST_CONVERTER(boolean, jboolean, int);
CAST_CONVERTER(byte, jbyte, int);
CAST_CONVERTER(char, jchar, int);
CAST_CONVERTER(double, jdouble, ufloat);
CAST_CONVERTER(float, jfloat, ufloat);
CAST_CONVERTER(int, jint, jint);
CAST_CONVERTER(long, jlong, jlong);
CAST_CONVERTER(short, jshort, int);

OBJECT_CONVERTER(UBinary, urbi::UBinary, "urbi/UBinary", "<init>", "(JZ)V");
OBJECT_CONVERTER(UDictionary, urbi::UDictionary, "urbi/UDictionary", "<init>", "(JZ)V");
OBJECT_CONVERTER(UImage, urbi::UImage, "urbi/UImage", "<init>", "(JZ)V");
OBJECT_CONVERTER(UList, urbi::UList, "urbi/UList", "<init>", "(JZ)V");
OBJECT_CONVERTER(USound, urbi::USound, "urbi/USound", "<init>", "(JZ)V");
OBJECT_CONVERTER(UValue, urbi::UValue, "urbi/UValue", "<init>", "(JZ)V");
OBJECT_CONVERTER(UVarBase, urbi::UVar, "urbi/UVar", "<init>", "(JZ)V");

PRIMITIVE_OBJECT_CONVERTER(Boolean, jboolean, int, "java/lang/Boolean", "valueOf", "(Z)Ljava/lang/Boolean;");
PRIMITIVE_OBJECT_CONVERTER(Byte, jbyte, int, "java/lang/Byte", "valueOf", "(B)Ljava/lang/Byte;");
PRIMITIVE_OBJECT_CONVERTER(Character, jchar, int, "java/lang/Character", "valueOf", "(C)Ljava/lang/Character;");
PRIMITIVE_OBJECT_CONVERTER(Double, jdouble, ufloat, "java/lang/Double", "valueOf", "(D)Ljava/lang/Double;");
PRIMITIVE_OBJECT_CONVERTER(Float, jfloat, ufloat, "java/lang/Float", "valueOf", "(F)Ljava/lang/Float;");
PRIMITIVE_OBJECT_CONVERTER(Integer, jint, jint, "java/lang/Integer", "valueOf", "(I)Ljava/lang/Integer;");
PRIMITIVE_OBJECT_CONVERTER(Long, jlong, jlong, "java/lang/Long", "valueOf", "(J)Ljava/lang/Long;");
PRIMITIVE_OBJECT_CONVERTER(Short, jshort, int, "java/lang/Short", "valueOf", "(S)Ljava/lang/Short;");

class UVarConverter : public UVarBaseConverter
{
protected:
  virtual urbi::UVar* alloc(const urbi::UValue& val)
  {
    urbi::UVar var =
      urbi::uvar_uvalue_cast<urbi::UVar&>(const_cast<urbi::UValue&>(val));
    return (urbi::UVar*)(allocated = new urbi::UVar(var));
  }
};

class UVarNotifyConverter : public UVarConverter
{
protected:
  jvalue convert_ (JNIEnv* env, urbi::UVar& val)
  {
    jobject jobj = env->NewObject(UVarConverter::cls,
				  UVarConverter::mid,
				  (jlong) &val,
				  false);
    allocationCheck(jobj, "UVar in notify");
    return *(jvalue*)&jobj;
  }
};

#define DECLARE_STATIC_ATTR_(Name)		\
  jclass Name##Converter::cls = 0;		\
  jmethodID Name##Converter::mid = 0

#define STATIC_ATTR_INIT_(Name, Env)		\
  Name##Converter::init(Env)

/// To be inserted in a .cc file
#define DECLARE_CONVERTERS_STATIC_ATTRS		\
  DECLARE_STATIC_ATTR_(Boolean);		\
  DECLARE_STATIC_ATTR_(Byte);			\
  DECLARE_STATIC_ATTR_(Character);		\
  DECLARE_STATIC_ATTR_(Double);			\
  DECLARE_STATIC_ATTR_(Float);			\
  DECLARE_STATIC_ATTR_(Integer);		\
  DECLARE_STATIC_ATTR_(Long);			\
  DECLARE_STATIC_ATTR_(Short);                  \
  DECLARE_STATIC_ATTR_(UBinary);		\
  DECLARE_STATIC_ATTR_(UDictionary);		\
  DECLARE_STATIC_ATTR_(UImage);			\
  DECLARE_STATIC_ATTR_(UList);			\
  DECLARE_STATIC_ATTR_(USound);			\
  DECLARE_STATIC_ATTR_(UValue);			\
  DECLARE_STATIC_ATTR_(UVarBase);

#define INIT_CONVERTERS_STATIC_ATTRS(env)	\
  STATIC_ATTR_INIT_(Boolean, env);		\
  STATIC_ATTR_INIT_(Byte, env);			\
  STATIC_ATTR_INIT_(Character, env);		\
  STATIC_ATTR_INIT_(Double, env);		\
  STATIC_ATTR_INIT_(Float, env);		\
  STATIC_ATTR_INIT_(Integer, env);		\
  STATIC_ATTR_INIT_(Long, env);			\
  STATIC_ATTR_INIT_(Short, env);                \
  STATIC_ATTR_INIT_(UBinary, env);		\
  STATIC_ATTR_INIT_(UDictionary, env);		\
  STATIC_ATTR_INIT_(UImage, env);		\
  STATIC_ATTR_INIT_(UList, env);		\
  STATIC_ATTR_INIT_(USound, env);		\
  STATIC_ATTR_INIT_(UValue, env);		\
  STATIC_ATTR_INIT_(UVarBase, env);

#endif

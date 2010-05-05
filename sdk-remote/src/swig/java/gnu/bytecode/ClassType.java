// Copyright (c) 1997, 1998, 1999, 2001, 2002, 2004, 2005  Per M.A. Bothner.
// This is free software;  for terms and warranty disclaimer see ./COPYING.

package gnu.bytecode;
import java.io.*;
import java.util.Vector;

public class ClassType extends ObjectType
  implements AttrContainer, Externalizable
{
  // An old but generally valid default value.
  int classfileFormatVersion = 45 * 0x10000 + 3;

  public short getClassfileMajorVersion ()
  {
    return (short) (classfileFormatVersion >> 16);
  }
  public short getClassfileMinorVersion ()
  {
    return (short) (classfileFormatVersion & 0xFFFF);
  }
  public void setClassfileVersion (int major, int minor)
  {
    classfileFormatVersion = (major & 0xFFFF) * 0x10000 + (minor * 0xFFFF);
  }
  public void setClassfileVersionJava5 ()
  {
    setClassfileVersion(49, 0);
  }

  /** Find a ClassType with the given name, or create a new one.
   * Use this for "library classes", where you need the field/method types,
   * but not one where you are about to generate code for.
   * @param name the name of the class (e..g. "java.lang.String").
   */
  public static ClassType make(String name)
  {
    return (ClassType) Type.getType(name);
  }

  public static ClassType make (String name, ClassType superClass)
  {
    ClassType type = make(name);
    if (type.superClass == null)
      type.setSuper(superClass);
    return type;
  }

  int thisClassIndex;

  /** The super (base) class of the current class.
   * X.superClass == null means the superClass has not been specified,
   * and defaults to java.lang.Object. */
  ClassType superClass;
  /** The constant pool index of the superClass, or -1 if unassigned. */
  int superClassIndex = -1;

  ClassType[] interfaces;
  int[] interfaceIndexes;
  public int access_flags;

  Attribute attributes;
  public final Attribute getAttributes () { return attributes; }
  public final void setAttributes (Attribute attributes)
    { this.attributes = attributes; }

  public static final ClassType[] noClasses = { };

  boolean emitDebugInfo = true;

  ConstantPool constants;

  public final ConstantPool getConstants () { return constants; }

  public final CpoolEntry getConstant(int i)
  {
    if (constants == null || constants.pool == null
	|| i > constants.count)
      return null;
    return constants.pool[i];
  }

  /** Return the modifiers (access flags) for this class. */
  public final int getModifiers()
  {
    if (access_flags == 0
	&& (flags & EXISTING_CLASS) != 0 && getReflectClass() != null)
      access_flags = reflectClass.getModifiers();
    return access_flags;
  }

  /** Set the modifiers (access flags) for this class. */
  public final void setModifiers(int flags) { access_flags = flags; }

  public final boolean hasOuterLink ()
  {
    getFields();
    return (flags & HAS_OUTER_LINK) != 0;
  }

  public ClassType getOuterLinkType ()
  {
    return ! hasOuterLink() ? null
      : (ClassType) getDeclaredField("this$0").getType();
  }

  /** Note that this class needs an other link ("this$0") field.
   * This is only allowed if !isExisting().
   * Adjust any existing "<init>" methods to take the extra
   * implicit parameter.
   * @param outer the outer class
   */
  public final Field setOuterLink (ClassType outer)
  {
    if ((flags & EXISTING_CLASS) != 0)
      throw new Error("setOuterLink called for existing class "+getName());
    Field field = getDeclaredField("this$0");
    if (field == null)
      {
        field = addField("this$0", outer);
        flags |= HAS_OUTER_LINK;
        for (Method meth = methods; meth != null;  meth = meth.getNext())
          {
            if ("<init>".equals(meth.getName()))
              {
                if (meth.code != null)
                  throw new Error("setOuterLink called when "+meth+" has code");
                Type[] arg_types = meth.arg_types;
                Type[] new_types = new Type[arg_types.length+1];
                System.arraycopy(arg_types, 0, new_types, 1, arg_types.length);
                new_types[0] = outer;
                meth.arg_types = new_types;
                meth.signature = null;
              }
          }
      }
    else if (! outer.equals(field.getType()))
      throw new Error("inconsistent setOuterLink call for "+getName());
    return field;
  }

  /** Check if a component is accessible from this class.
   * @param declaring the class containing the component (a field, method,
   *   or inner class)
   * @param modifiers the access flags of the component
   * @return true if the specified component can be accessed from this class.
   */
  public boolean isAccessible (ClassType declaring, int modifiers)
  {
    int cmods = declaring.getModifiers();
    if ((modifiers & Access.PUBLIC) != 0 && (cmods & Access.PUBLIC) != 0)
      return true;
    String callerName = getName();
    String className = declaring.getName();
    if (callerName.equals(className))
      return true;
    if ((modifiers & Access.PRIVATE) != 0)
      return false;
    int dot = callerName.lastIndexOf('.');
    String callerPackage = dot >= 0 ? callerName.substring(0, dot) : "";
    dot = className.lastIndexOf('.');
    String classPackage = dot >= 0 ? className.substring(0, dot) : "";
    if (callerPackage.equals(classPackage))
      return true;
    if ((modifiers & Access.PROTECTED) != 0
        && this.isSubclass(declaring))
      return true;
    return false;
  }

  /** Sets the name of the class being defined in this classfile.
   * @param name the name to give to the class
   */
  public void setName (String name)
  {
    this_name = name;
    setSignature("L"+name.replace('.', '/')+";");
  }

  SourceDebugExtAttr sourceDbgExt;

  /** Create a <code>SourceDebugExtAttr</code>, if needed, and
   * set the "stratum".  The stratum is typically a programming language
   * such as "JSP", "Scheme", or "Java" (the default). */
  public void setStratum (String stratum)
  {
    if (sourceDbgExt == null)
      sourceDbgExt = new SourceDebugExtAttr(this);
    sourceDbgExt.addStratum(stratum);
  }

  /** Set the name of the SourceFile associated with this class. */
  public void setSourceFile (String name)
  {
    if (sourceDbgExt != null)
      {
	sourceDbgExt.addFile(name);
	if (sourceDbgExt.fileCount > 1)
	  return;
      }

    name = SourceFileAttr.fixSourceFile(name);
    int slash = name.lastIndexOf('/');
    if (slash >= 0)
      name = name.substring(slash+1);
    SourceFileAttr.setSourceFile(this, name);
  }

  /**
   * Set the superclass of the is class.
   * @param name name of super class, or null if this is "Object".
   */
  public void setSuper (String name)
  {
    setSuper(name == null ? Type.pointer_type : ClassType.make(name));
  }

  public void setSuper (ClassType superClass)
  {
    this.superClass = superClass;
  }

  public ClassType getSuperclass ()
  {
    if (superClass == null
	&& ! isInterface()
	&& ! ("java.lang.Object".equals(getName()))
	&& (flags & EXISTING_CLASS) != 0 && getReflectClass() != null)
      {
	superClass = (ClassType) make(reflectClass.getSuperclass());
      }
    return superClass;
  }

  public String getPackageName()
  {
    String name = getName();
    int index = name.lastIndexOf('.');
    return index < 0 ? "" : name.substring(0, index);
  }

  /**
   * @return the interfaces this class is declared to implement
   * (not those inherited from its superclass/superinterfaces).
   */
  public synchronized ClassType[] getInterfaces()
  {
    if (interfaces == null
	&& (flags & EXISTING_CLASS) != 0 && getReflectClass() != null)
      {
	Class[] reflectInterfaces = reflectClass.getInterfaces();
	int numInterfaces = reflectInterfaces.length;
	interfaces
	  = numInterfaces == 0 ? noClasses : new ClassType[numInterfaces];

	for (int i = 0; i < numInterfaces; i++)
	  interfaces[i] = (ClassType) Type.make(reflectInterfaces[i]);
      }
    return interfaces;
  }

  public void setInterfaces (ClassType[] interfaces)
  { this.interfaces = interfaces; }

  /** Add an interface to the list of implemented interfaces. */
  public void addInterface (ClassType newInterface)
  {
    int oldCount;
    if (interfaces == null || interfaces.length == 0)
      {
	oldCount = 0;
	interfaces = new ClassType[1];
      }
    else
      {
	oldCount = interfaces.length;
	for (int i = oldCount;  --i >= 0; )
	  if (interfaces[i] == newInterface)
	    return;
	ClassType[] newInterfaces = new ClassType[oldCount+1];
	System.arraycopy(interfaces, 0, newInterfaces, 0, oldCount);
	interfaces = newInterfaces;
      }
    interfaces[oldCount] = newInterface;
  }

  public final boolean isInterface()
  { return (getModifiers() & Access.INTERFACE) != 0; }

  public final void setInterface(boolean val)
  {
    if (val) access_flags |= Access.INTERFACE|Access.ABSTRACT;
    else access_flags &= ~Access.INTERFACE|Access.ABSTRACT;
  }

  public ClassType () { }

  public ClassType (String class_name)
  {
    super();
    setName(class_name);
  }

  Field fields;
  int fields_count;
  Field last_field;
  /**  Constant pool index of "ConstantValue". */
  int ConstantValue_name_index;

  /** Constant pool index of "Code". */
  int Code_name_index;

  /** Constant pool index of "LocalVariableTable". */
  int LocalVariableTable_name_index;

  /** Constant pool index of "LineNumberTable". */
  int LineNumberTable_name_index;

  /** Get the fields of this class. */
  public final synchronized Field getFields()
  {
    if ((flags & (ADD_FIELDS_DONE|EXISTING_CLASS)) == EXISTING_CLASS)
      addFields();
    return fields;
  }

  public final int getFieldCount()
  {
    return fields_count;
  }

  /** Find a field with the given name declared in this class.
   * @return the matching field, or null if there is no such field.
   */
  public Field getDeclaredField(String name)
  {
    for (Field field = getFields();   field != null;  field = field.next)
      {
	if (name.equals(field.name))
	  return field;
      }
    return null;
  }

  /** Find a field with the given name declared in this class or its ancestors.
   * @param name the name of the field.
   * @param mask of match a field whose modifiers has one of these bits set.
   *   Howeve, if mask is -1, ignore the access flags.
   * @return the matching field, or null if there is no such field.
   */
  public Field getField(String name, int mask)
  {
    ClassType cl = this;
    for (;;)
      {
        Field field = cl.getDeclaredField(name);
        if (field != null
            && (mask == -1 || (field.getModifiers() & mask) != 0))
          return field;
        ClassType[] interfaces = cl.getInterfaces();
        if (interfaces != null)
          {
            for (int i = 0;  i < interfaces.length;  i++)
              {
                field = interfaces[i].getField(name, mask);
                if (field != null)
                  return field;
              }
          }
        cl = cl.getSuperclass();
        if (cl == null)
          return null;
      }
  }

  /** Find a field with the given name declared in this class or its ancestors.
   * @return the matching field, or null if there is no such field.
   */
  public Field getField(String name)
  {
    return getField(name, Access.PUBLIC);
  }

  /**
   * Add a new field to this class.
   */
  public Field addField () { return new Field (this); }

  /**
   * Add a new field to this class, and name the field.
   * @param name the name of the new field
   */
  public Field addField (String name) {
    Field field = new Field (this);
    field.setName(name);
    return field;
  }

  public final Field addField (String name, Type type) {
    Field field = new Field (this);
    field.setName(name);
    field.setType(type);
    return field;
  }

  public final Field addField (String name, Type type, int flags)
  {
    Field field = addField (name, type);
    field.flags = flags;
    return field;
  }

  /** Use reflection to add all the declared fields of this class.
   * Does not add private or package-private fields.
   * Does not check for duplicate (already-known) fields.
   * Is not thread-safe if another thread may access this ClassType. */
  public void addFields()
  {
    Class clas = getReflectClass();
    java.lang.reflect.Field[] fields;
    try
      {
        fields = clas.getDeclaredFields();
      }
    catch (SecurityException ex)
      {
        fields = clas.getFields();
      }
    int count = fields.length;
    for (int i = 0;  i < count;  i++)
      {
        java.lang.reflect.Field field = fields[i];
        if ("this$0".equals(field.getName()))
          flags |= HAS_OUTER_LINK;
        addField(field.getName(), Type.make(field.getType()),
                 field.getModifiers());
      }
    flags |= ADD_FIELDS_DONE;
  }

  Method methods;
  int methods_count;
  Method last_method;
  public Method constructor;

  /** Get the methods of this class. */
  public final Method getMethods()
  {
    return methods;
  }

  public final int getMethodCount() {
    return methods_count;
  }

  Method addMethod () {
    return new Method (this, 0);
  }

  public Method addMethod (String name) {
    return addMethod(name, 0);
  }

  public Method addMethod (String name, int flags) {
    Method method = new Method (this, flags);
    method.setName(name);
    return method;
  }

  // deprecated:
  public Method addMethod (String name,
			   Type[] arg_types, Type return_type,
			   int flags) {
    return addMethod(name, flags, arg_types, return_type);
  }

  /** Add a method to this ClassType.
    * If an existing method matches, return that.  Otherwise, create
    * a new one.
    * In contrast, the other addMethod methods always create new Methods. */
  public Method addMethod (String name, int flags,
			   Type[] arg_types, Type return_type)
  {
    Method method = getDeclaredMethod(name, arg_types);
    if (method != null
        && return_type.equals(method.getReturnType())
        && (flags & method.access_flags) == flags)
      return method;
    method = addMethod(name, flags);
    method.arg_types = arg_types;
    method.return_type = return_type;
    return method;
  }

  public Method addMethod (String name,  String signature, int flags)
  {
    Method meth = addMethod(name, flags);
    meth.setSignature(signature);
    return meth;
  }

  /** Add a method to this ClassType.
    * If an existing method matches, return that.  Otherwise, create
    * a new one. */
  public Method getMethod (java.lang.reflect.Method method)
  {
    String name = method.getName();
    Class[] parameterClasses = method.getParameterTypes();
    Type[] parameterTypes = new Type[parameterClasses.length];
    for (int i = parameterClasses.length;  --i >= 0; )
      parameterTypes[i] = Type.make(parameterClasses[i]);
    return addMethod(name, method.getModifiers(),
                     parameterTypes, Type.make(method.getReturnType()));
  }

  public final synchronized Method getDeclaredMethods()
  {
    if ((flags & (ADD_METHODS_DONE|EXISTING_CLASS)) == EXISTING_CLASS)
      addMethods(getReflectClass());
    return methods;
  }

  /** Count methods matching a given filter.
   * @param filter to select methods to return
   * @param searchSupers 0 if only current class should be searched,
   *   1 if superclasses should also be searched,
   *   2 if super-interfaces should also be search
   * @return number of methods that match
   */
  public final int countMethods (Filter filter, int searchSupers)
  {
    return getMethods(filter, searchSupers, null, 0);
  }

  public Method[] getMethods (Filter filter, boolean searchSupers)
  {
    return getMethods(filter, searchSupers ? 1 : 0);
  }

  /** Get methods matching a given filter.
   * @param filter to select methods to return
   * @param searchSupers 0 if only current class should be searched,
   *   1 if superclasses should also be searched,
   *   2 if super-interfaces should also be searched
   * @return a fresh array containing the methods satisfying the filter
   */
  public Method[] getMethods (Filter filter, int searchSupers)
  {
    int count = getMethods(filter, searchSupers, null, 0);
    Method[] result = new Method[count];
    getMethods(filter, searchSupers, result, 0);
    return result;
  }

  /** Helper to get methods satisfying a filtering predicate.
   * @param filter to select methods to return
   * @param searchSupers 0 if only current class should be searched,
   *   1 if superclasses should also be searched,
   *   2 if super-interfaces should also be searched
   * @param result array to place selected methods in
   * @param offset start of where in result to place result
   * @return number of methods placed in result array
   * @deprecated
   */
  public int getMethods (Filter filter, int searchSupers,
			 Method[] result, int offset)
  {
    int count = 0;
    for (ClassType ctype = this;  ctype != null;
	 ctype = ctype.getSuperclass())
    {
      for (Method meth = ctype.getDeclaredMethods();
	   meth != null;  meth = meth.getNext())
	if (filter.select(meth))
	  {
	    if (result != null)
	      result[offset + count] = meth;
	    count++;
	  }
      if (searchSupers == 0)
	break;

      if (searchSupers > 1)
	{
	  ClassType[] interfaces = ctype.getInterfaces();
	  if (interfaces != null)
	    {
	      for (int i = 0;  i < interfaces.length;  i++)
		count += interfaces[i].getMethods(filter, searchSupers,
						  result, offset+count);
	    }
	}
    }
    return count;
  }

  /** Helper to get methods satisfying a filtering predicate.
   * @param filter to select methods to return
   * @param searchSupers 0 if only current class should be searched,
   *   1 if superclasses should also be searched,
   *   2 if super-interfaces should also be searched
   * @param result Vector to add selected methods in
   * @param context If non-null, skip if class not visible in named package.
   * @return number of methods placed in result array
   */
  public int getMethods (Filter filter, int searchSupers, Vector result,
			 String context)
  {
    int count = 0;
    for (ClassType ctype = this;  ctype != null;
	 ctype = ctype.getSuperclass())
    {
      if (context == null
	  || (ctype.getModifiers() & Access.PUBLIC) != 0
	  || context.equals(ctype.getPackageName()))
	{
	  for (Method meth = ctype.getDeclaredMethods();
	       meth != null;  meth = meth.getNext())
	    if (filter.select(meth))
	      {
		if (result != null)
		  result.addElement(meth);
		count++;
	      }
	}
      if (searchSupers == 0)
	break;

      if (searchSupers > 1)
	{
	  ClassType[] interfaces = ctype.getInterfaces();
	  if (interfaces != null)
	    {
	      for (int i = 0;  i < interfaces.length;  i++)
		count += interfaces[i].getMethods(filter, searchSupers,
						  result, context);
	    }
	}
    }
    return count;
  }

  /** Look for a matching method.
   * @param name method name
   * @param arg_types parameter types that must match.
   *  Can also be null, to match any parameter type list.
   *  Otherwise, an element of arg_types must be the same type (equals),
   *  though a null element of arg_types is a wildcard that matches any type.
   */
  public Method getDeclaredMethod(String name, Type[] arg_types)
  {
    int needOuterLinkArg = "<init>".equals(name) && hasOuterLink() ? 1 : 0;
    for (Method method = getDeclaredMethods();
	 method != null;  method = method.next)
      {
	if (! name.equals(method.getName()))
	  continue;
	Type[] method_args = method.getParameterTypes();
	if (arg_types == null
            || (arg_types == method_args && needOuterLinkArg==0))
	  return method;
	int i = arg_types.length;
	if (i != method_args.length-needOuterLinkArg)
	  continue;
	while (-- i >= 0)
	  {
	    Type meth_type = method_args[i+needOuterLinkArg];
	    Type need_type = arg_types[i];
	    if (meth_type == need_type || need_type == null)
	      continue;
	    String meth_sig = meth_type.getSignature();
	    String need_sig = need_type.getSignature();
	    if (! meth_sig.equals(need_sig))
	      break;
	  }
	if (i < 0)
	  return method;
      }
    return null;
  }

  /** Get a method with matching name and number of arguments. */
  public Method getDeclaredMethod(String name, int argCount)
  {
    Method result = null;
    int needOuterLinkArg = "<init>".equals(name) && hasOuterLink() ? 1 : 0;
    for (Method method = getDeclaredMethods();
	 method != null;  method = method.next)
      {
	if (name.equals(method.getName())
	    && argCount + needOuterLinkArg == method.getParameterTypes().length)
	  {
	    if (result != null)
	      throw new Error("ambiguous call to getDeclaredMethod(\""
			      + name + "\", " + argCount+
			      ")\n - " + result + "\n - " + method);
	    result = method;
	  }
      }
    return result;
  }

  public Method getMethod(String name, Type[] arg_types)
  {
    ClassType cl = this;
    for (;;)
      {
        Method method = cl.getDeclaredMethod(name, arg_types);
	if (method != null)
          return method;
        cl = cl.getSuperclass();
        if (cl == null)
          break;
      }
    cl = this;
    for (;;)
      {
        ClassType[] interfaces = cl.getInterfaces();
        if (interfaces != null)
          {
            for (int i = 0;  i < interfaces.length;  i++)
              {
                Method method
                  = interfaces[i].getDeclaredMethod(name, arg_types);
                if (method != null)
                  return method;
              }
          }
        cl = cl.getSuperclass();
        if (cl == null)
          break;
      }
    return null;
  }

  /** Use reflection to add all the declared methods of this class.
   * Does not add constructors nor private or package-private methods.
   * Does not check for duplicate (already-known) methods.
   * @param clas should be the same as getReflectClass(). */
  public void addMethods(Class clas)
  {
    // Set this flag BEFORE the actual addition.
    // This prevents this method to be called indirectly for the same class
    // while it is executed, which would result in methods being listed
    // twice in this class.
    flags |= ADD_METHODS_DONE;

    java.lang.reflect.Method[] methods;
    try
      {
        methods = clas.getDeclaredMethods();
      }
    catch (SecurityException ex)
      {
        methods = clas.getMethods();
      }
    int count = methods.length;
    for (int i = 0;  i < count;  i++)
      {
        java.lang.reflect.Method method = methods[i];
        if (! method.getDeclaringClass().equals(clas))
          continue;
        int modifiers = method.getModifiers();
        Class[] paramTypes = method.getParameterTypes();
        int j = paramTypes.length;
        Type[] args = new Type[j];
        while (--j >= 0)
          args[j] = Type.make(paramTypes[j]);
        Method meth = addMethod(method.getName(), modifiers);
        meth.arg_types = args;
        meth.return_type = Type.make(method.getReturnType());
      }

    java.lang.reflect.Constructor[] cmethods;
    try
      {
        cmethods = clas.getDeclaredConstructors();
      }
    catch (SecurityException ex)
      {
        cmethods = clas.getConstructors();
      }
    count = cmethods.length;
    for (int i = 0;  i < count;  i++)
      {
        java.lang.reflect.Constructor method = cmethods[i];
        if (! method.getDeclaringClass().equals(clas))
          continue;
        int modifiers = method.getModifiers();
        if ((modifiers & (Access.PUBLIC|Access.PROTECTED)) == 0)
          continue;
        Class[] paramTypes = method.getParameterTypes();
        int j = paramTypes.length;
        Type[] args = new Type[j];
        while (--j >= 0)
          args[j] = Type.make(paramTypes[j]);
        Method meth = addMethod("<init>", modifiers);
        meth.arg_types = args;
        meth.return_type = Type.void_type;
      }
  }

  public Method[] getMatchingMethods(String name, Type[] paramTypes, int flags)
  {
    int nMatches = 0;
    java.util.Vector matches = new java.util.Vector(10);
    for (Method method = methods;  method != null;  method = method.getNext())
    {
      if (! name.equals(method.getName()))
        continue;
      if ((flags & Access.STATIC) != (method.access_flags & Access.STATIC))
        continue;
      if ((flags & Access.PUBLIC) > (method.access_flags & Access.PUBLIC))
        continue;
      Type[] mtypes = method.arg_types;
      if (mtypes.length != paramTypes.length)
        continue;
      nMatches++;
      matches.addElement(method);
    }
    Method[] result = new Method[nMatches];
    matches.copyInto(result);
    return result;
  }

  /** Do various fixups after generating code but before we can write it out.
   * This includes assigning constant pool indexes where needed,
   * finalizing labels, etc. */
  public void doFixups ()
  {
    if (constants == null)
      constants = new ConstantPool();
    if (thisClassIndex == 0)
      thisClassIndex = constants.addClass(this).index;
    if (superClass == this)
      setSuper((ClassType) null);
    if (superClassIndex < 0)
      superClassIndex = superClass == null ? 0
	: constants.addClass(superClass).index;
    if (interfaces != null && interfaceIndexes == null)
      {
	int n = interfaces.length;
	interfaceIndexes = new int [n];
	for (int i = 0;  i < n;  i++)
	  interfaceIndexes[i] = constants.addClass(interfaces[i]).index;
      }
    for (Field field = fields; field != null; field = field.next) {
      field.assign_constants (this);
    }
    for (Method method = methods; method != null; method = method.next)
      method.assignConstants();
    Attribute.assignConstants(this, this);
  }

  public void writeToStream (OutputStream stream)
    throws java.io.IOException
  {
    java.io.DataOutputStream dstr = new java.io.DataOutputStream (stream);
    int i;

    doFixups ();

    dstr.writeInt (0xcafebabe);  // magic
    dstr.writeShort(getClassfileMinorVersion());
    dstr.writeShort(getClassfileMajorVersion());

    // Write out the constant pool.
    if (constants == null)
      dstr.writeShort (1);
    else
      constants.write(dstr);

    dstr.writeShort (access_flags);
    dstr.writeShort (thisClassIndex);
    dstr.writeShort (superClassIndex);
    if (interfaceIndexes == null)
      dstr.writeShort (0);  // interfaces_count
    else
      {
	int interfaces_count = interfaceIndexes.length;
	dstr.writeShort (interfaces_count);
	for (i = 0;  i < interfaces_count; i++)
	  dstr.writeShort (interfaceIndexes[i]);
      }

    dstr.writeShort (fields_count);
    for (Field field = fields;  field != null;  field = field.next)
      field.write (dstr, this);

    dstr.writeShort (methods_count);
    for (Method method = methods;  method != null;  method = method.next)
      method.write (dstr, this);

    Attribute.writeAll (this, dstr);

    flags |= ADD_FIELDS_DONE | ADD_METHODS_DONE;
  }

  public void writeToFile (String filename)
    throws java.io.IOException
 {
    OutputStream stream
      = new BufferedOutputStream(new FileOutputStream (filename));
    writeToStream (stream);
    stream.close ();
  }

  public void writeToFile ()
    throws java.io.IOException
  {
    writeToFile (this_name.replace ('.', File.separatorChar) + ".class");
  }

  public byte[] writeToArray ()
  {
    ByteArrayOutputStream stream = new ByteArrayOutputStream (500);
    try
      {
	writeToStream(stream);
      }
    catch (java.io.IOException ex)
      {
	throw new InternalError(ex.toString());
      }
    return stream.toByteArray ();
  }

  /**
   * Convert a String to a Utf8 byte array.
   * @param str the input String.
   * @return the input encoded as a utf8 byte array.
   */
  public static byte[] to_utf8 (String str)
  {
    if (str == null)
      return null;
    int str_len = str.length ();
    int utf_len = 0;
    for (int i = 0; i < str_len; i++) {
      int c = str.charAt(i);
      if ((c > 0) && (c <= 0x7F))
	utf_len++;
      else if (c <= 0x7FF)
	utf_len += 2;
      else
	utf_len += 3;
    }
    byte[] buffer = new byte[utf_len];
    int j = 0;
    for (int i = 0; i < str_len; i++) {
      int c = str.charAt(i);
      if ((c > 0) && (c <= 0x7F))
	buffer[j++] = (byte) c;
      else if (c <= 0x7FF) {
	buffer[j++] = (byte) (0xC0 | ((c >>  6) & 0x1F));
	buffer[j++] = (byte) (0x80 | ((c >>  0) & 0x3F));
      } else {
	buffer[j++] = (byte) (0xE0 | ((c >> 12) & 0x0F));
	buffer[j++] = (byte) (0x80 | ((c >>  6) & 0x3F));
	buffer[j++] = (byte) (0x80 | ((c >>  0) & 0x3F));
      }
    }
    return buffer;
  }

  /** True if this class/interface implements the interface iface. */
  public final boolean implementsInterface(ClassType iface)
  {
    if (this == iface)
      return true;
    ClassType baseClass = this.getSuperclass();
    if (baseClass != null && baseClass.implementsInterface(iface))
      return true;
    ClassType[] interfaces = getInterfaces();
    if (interfaces != null)
      {
	for (int i = interfaces.length;  --i >= 0; )
	  {
	    if (interfaces[i].implementsInterface(iface))
	      return true;
	  }
      }
    return false;
  }

  /** A more efficient version of isSubclass(ClassType.make(cname)).
   * Does not cause the named class be loaded if it hasn't been.
   * @param cname a class name - cannot be an interface name
   */
  public final boolean isSubclass (String cname)
  {
    ClassType ctype = this;
    for (;;)
      {
        if (cname.equals(ctype.getName()))
          return true;
        ctype = ctype.getSuperclass();
        if (ctype == null)
          return false;
      }
  }

  public final boolean isSubclass(ClassType other)
  {
    if (other.isInterface())
      return implementsInterface(other);
    if ((this == tostring_type && other == string_type)
	|| (this == string_type && other == tostring_type))
      return true;
    ClassType baseClass = this;
    while (baseClass != null)
      {
        if (baseClass == other)
          return true;
        baseClass = baseClass.getSuperclass();
      }
    return false;
  }

  public int compare(Type other)
  {
    if (other == nullType)
      return 1;
    if (! (other instanceof ClassType))
      return swappedCompareResult(other.compare(this));
    String name = getName();
    if (name != null && name.equals(other.getName()))
      return 0;
    ClassType cother = (ClassType) other;
    if (isSubclass(cother))
      return -1;
    if (cother.isSubclass(this))
      return 1;
    if (this == tostring_type)
      return cother == Type.pointer_type ? -1 : 1;
    if (cother == tostring_type)
      return this == Type.pointer_type ? 1 : -1;
    if (this.isInterface() || cother.isInterface())
      return -2;
    return -3;
  }

  public String toString()
  {
    return "ClassType " + getName();
  }

  /**
   * @serialData Write the class name (as given by getName()) using writeUTF.
   */
  public void writeExternal(ObjectOutput out) throws IOException
  {
    out.writeUTF(getName());
  }

  public void readExternal(ObjectInput in)
    throws IOException, ClassNotFoundException
  {
    setName(in.readUTF());
    flags |= ClassType.EXISTING_CLASS;
  }

  public Object readResolve() throws ObjectStreamException
  {
    String name = getName();
    /* #ifdef JAVA5 */
    // java.util.HashMap<String,Type> map = mapNameToType;
    /* #else */
    java.util.Hashtable map = mapNameToType;
    /* #endif */
    synchronized (map)
      {
        Type found = (Type) map.get(name);
        if (found != null)
          return found;
        map.put(name, this);
      }
    return this;
  }

  /** Clear various object references, to help garbage collection. */
  public void cleanupAfterCompilation ()
  {
    for (Method meth = methods;  meth != null;  meth = meth.getNext())
      meth.cleanupAfterCompilation();

    constants = null;
    attributes = null;
    sourceDbgExt = null;
  }
}

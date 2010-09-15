/**
 *
 * Copyright (C) Gostai S.A.S., 2006,2007,2008.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 * For comments, bug reports and feedback: http://www.urbiforge.com
 */


package urbi;

import gnu.bytecode.*;
import java.lang.Class;
import java.lang.reflect.Method;
import java.lang.reflect.Constructor;
import java.lang.RuntimeException;
import java.util.LinkedList;
import urbi.generated.urbi;
import urbi.generated.UObjectCPP;
import urbi.generated.UVar;
import urbi.generated.URBIStarterJAVA;
import urbi.generated.UContextImpl;
import urbi.generated.UValue;
import urbi.generated.UrbiRoot;

/// This class is the base UObject class for Java UObject.
/// Please have all your Java UObject extends UObject
public class UObject extends UObjectCPP
{

    /// Constructor
    public UObject (String s) {
	super (s);
    }


    /// ------------------ ///
    ///                    ///
    ///  UNotifyOnRequest  ///
    ///                    ///
    /// ------------------ ///

    protected native void registerNotifyOnRequest(long var,
						  String var_name,
						  boolean is_owned,
						  String obj_name,
						  String method,
						  String signature,
						  int    arg_number);


    protected void UNotifyOnRequest (String var_name, Method m)
    {
	checkNotifyRegisteredMethod (m, "UNotifyOnRequest");
	String bytecode_sig = getMethodBytecodeSignature (m);
	registerNotifyOnRequest (0,
				 var_name,
				 false,
				 get__name (),
				 m.getName (),
				 bytecode_sig,
				 m.getParameterTypes().length);
    }

    protected void UNotifyOnRequest (UVar v, Method m)
    {
	checkNotifyRegisteredMethod (m, "UNotifyOnRequest");
	String bytecode_sig = getMethodBytecodeSignature (m);
	registerNotifyOnRequest (UVar.getCPtr (v),
				 v.getName(),
				 v.getOwned (),
				 get__name (),
				 m.getName (),
				 bytecode_sig,
				 m.getParameterTypes().length);
    }

    protected void UNotifyOnRequest (String var_name, String method_name)
    {
	Method m = findMethodFromName (this, method_name);
	UNotifyOnRequest (var_name, m);
    }

    protected void UNotifyOnRequest (String var_name,
				     String method_name,
				     String[] parameters_name)
    {
	Method m;
	try {
	    Class[] params = stringTypeToClassType (parameters_name);
	    Class obj_class = getClass ();
	    m = obj_class.getMethod (method_name, params);
	}
	catch (java.lang.ClassNotFoundException e) {
	    throw new RuntimeException (e);
	}
	catch (java.lang.NoSuchMethodException e) {
	    throw new RuntimeException (e);
        }
	UNotifyOnRequest (var_name, m);
    }


    protected void UNotifyOnRequest (UVar v, String method_name)
    {
	Method m = findMethodFromName (this, method_name);
	UNotifyOnRequest (v, m);
    }

    protected void UNotifyOnRequest (UVar v,
				     String method_name,
				     String[] parameters_name)
    {
	Method m;
	try {
	    Class[] params = stringTypeToClassType (parameters_name);
	    Class obj_class = getClass ();
	    m = obj_class.getMethod (method_name, params);
	}
	catch (java.lang.ClassNotFoundException e) {
	    throw new RuntimeException (e);
	}
	catch (java.lang.NoSuchMethodException e) {
	    throw new RuntimeException (e);
        }
	UNotifyOnRequest (v, m);
    }


    /// --------------- ///
    ///                 ///
    ///  UNotifyChange  ///
    ///                 ///
    /// --------------- ///

    protected native void registerNotifyChange(long var,
					       String var_name,
					       boolean is_owned,
					       String obj_name,
					       String method,
					       String signature,
					       int    arg_number);

    protected void UNotifyChange (String var_name, Method m)
    {
	checkNotifyRegisteredMethod (m, "UNotifyChange");
	String bytecode_sig = getMethodBytecodeSignature (m);
	registerNotifyChange (0,
			      var_name,
			      false,
			      get__name (),
			      m.getName (),
			      bytecode_sig,
			      m.getParameterTypes().length);
    }

    protected void UNotifyChange (UVar v, Method m)
    {
	checkNotifyRegisteredMethod (m, "UNotifyChange");
	String bytecode_sig = getMethodBytecodeSignature (m);
	registerNotifyChange (UVar.getCPtr (v),
			      v.getName(),
			      v.getOwned (),
			      get__name (),
			      m.getName (),
			      bytecode_sig,
			      m.getParameterTypes().length);
    }

    protected void UNotifyChange (String var_name, String method_name)
    {
	Method m = findMethodFromName (this, method_name);
	UNotifyChange (var_name, m);
    }

    protected void UNotifyChange (String var_name,
				  String method_name,
				  String[] parameters_name)
    {
	Method m;
	try {
	    Class[] params = stringTypeToClassType (parameters_name);
	    Class obj_class = getClass ();
	    m = obj_class.getMethod (method_name, params);
	}
	catch (java.lang.ClassNotFoundException e) {
	    throw new RuntimeException (e);
	}
	catch (java.lang.NoSuchMethodException e) {
	    throw new RuntimeException (e);
        }
	UNotifyChange (var_name, m);
    }


    protected void UNotifyChange (UVar v, String method_name)
    {
	Method m = findMethodFromName (this, method_name);
	UNotifyChange (v, m);
    }

    protected void UNotifyChange (UVar v,
				  String method_name,
				  String[] parameters_name)
    {
	Method m;
	try {
	    Class[] params = stringTypeToClassType (parameters_name);
	    Class obj_class = getClass ();
	    m = obj_class.getMethod (method_name, params);
	}
	catch (java.lang.ClassNotFoundException e) {
	    throw new RuntimeException (e);
	}
	catch (java.lang.NoSuchMethodException e) {
	    throw new RuntimeException (e);
        }
	UNotifyChange (v, m);
    }



    /// --------------- ///
    ///                 ///
    ///  UBindFunction  ///
    ///                 ///
    /// --------------- ///

    protected native void registerFunction (Object obj,
					    String obj_name,
					    String method,
					    String signature,
					    String return_type,
					    int    arg_number);

    protected void UBindFunction (Object obj, Method m)
    {
	Class[] p = m.getParameterTypes();

	if (p.length > 16) {

	    String msg = "Function \""+ m.getName () + "\" has " + p.length
		+ " arguments. You can't bind a function with more than "
		+ "16 arguments with UBindFunction.";
	    throw new RuntimeException (msg);
	}

	for (int i = 0; i < p.length; ++i) {

	    if (p[i] != UValue.class) {
		String msg = "Parameter " + (i + 1) + " of the function \""
		    + m.getName () + "\" is of type "+ p[i].getName ()
		    + ". You can only bind functions with parameters of"
		    + " type liburbi.main.UValue with UBindFunction.";
		throw new RuntimeException (msg);
	    }

	}

	String bytecode_sig = getMethodBytecodeSignature (m);
	registerFunction (obj,
			  get__name (),
			  m.getName (),
			  bytecode_sig,
			  m.getReturnType().getName (),
			  m.getParameterTypes().length);
    }

    protected void UBindFunction (Object obj,
				  String method_name)
    {
	Method m = findMethodFromName (obj, method_name);
	UBindFunction (obj, m);
    }



    protected void UBindFunction (Object obj,
				  String method_name,
				  String[] parameters_name)
    {
	Method m;
	try {
	    Class[] params = stringTypeToClassType (parameters_name);
	    Class obj_class = obj.getClass ();
	    m = obj_class.getMethod (method_name, params);
	}
	catch (java.lang.ClassNotFoundException e) {
	    throw new RuntimeException (e);
	}
	catch (java.lang.NoSuchMethodException e) {
	    throw new RuntimeException (e);
        }
	UBindFunction (obj, m);
    }

    /// --------------- ///
    ///                 ///
    ///    USetTimer    ///
    ///                 ///
    /// --------------- ///

    protected native void registerTimerFunction (Object obj,
						 String obj_name,
						 double period,
						 String method,
						 String signature,
						 String return_type,
						 int    arg_number);

    protected void USetTimer (double period, Object obj, Method m)
	throws RuntimeException
    {
	Class[] p = m.getParameterTypes();
	if (p.length > 0) {

	    String msg = "Function \""+ m.getName () + "\" has " + p.length
		+ " arguments. You can only register functions with zero "
		+ " arguments with USetTimer.";
	    throw new RuntimeException (msg);
	}
	if (m.getReturnType() != int.class) {
	    String msg = "Return type of the function \""
		+ m.getName () + "\" is "+ m.getReturnType().getName ()
		+ ". You can only register functions with a return type of"
		+ " type int with USetTimer.";
	    throw new RuntimeException (msg);
	}

	String bytecode_sig = getMethodBytecodeSignature (m);
	registerTimerFunction (obj,
			       get__name (),
			       period,
			       m.getName (),
			       bytecode_sig,
			       m.getReturnType().getName (),
			       m.getParameterTypes().length);
    }


    protected void USetTimer (double period,
			      Object obj,
			      String method_name,
			      String[] parameters_name)
    {
	Method m;
	try {
	    Class[] params = stringTypeToClassType (parameters_name);
	    Class obj_class = obj.getClass ();
	    m = obj_class.getMethod (method_name, params);
	}
	catch (java.lang.ClassNotFoundException e) {
	    throw new RuntimeException (e);
	}
	catch (java.lang.NoSuchMethodException e) {
	    throw new RuntimeException (e);
        }
	USetTimer (period, obj, m);
    }

    protected void USetTimer (double period,
			      Object obj,
			      String method_name)
    {
	Method m = findMethodFromName (obj, method_name);
	USetTimer (period, obj, m);
    }

    /// --------------- ///
    ///                 ///
    ///     UBindVar    ///
    ///                 ///
    /// --------------- ///

    protected void UBindVar (UVar v, String name) {
	v.init (get__name (), name);
    }


    /// --------------- ///
    ///                 ///
    ///      UStart     ///
    ///                 ///
    /// --------------- ///

    public static void UStart (Class uobject_cls)
    {
	String name = uobject_cls.getName ();
	/// Get the constructor that take a string
	Class string_cls;
	Constructor uobject_ctor;
	try {
	    string_cls = Class.forName ("java.lang.String");
	    Class[] params = {string_cls};
	    uobject_ctor = uobject_cls.getConstructor (params);
	}
	catch (java.lang.ClassNotFoundException e) {
	    throw new RuntimeException (e);
	}
	catch (java.lang.NoSuchMethodException e) {
	    throw new RuntimeException (e);
	}
	String[] urbi_name = name.split ("\\.");
	if (urbi_name.length > 0) {
	    starterList.add (new UObjectStarter (urbi_name[urbi_name.length - 1], uobject_ctor));
	}
    }


    public static void main (String argv[]) {
	/// Add program name at the begining of the argv array
	String[] new_argv = new String [argv.length + 1];
	for (int i = 0; i < argv.length; ++i) {
	    new_argv[i + 1] = argv[i];
	}
	new_argv[0] = "UObject";
	UrbiRoot root = new UrbiRoot("urbi-launch", false);
	urbi.main (new_argv.length, new_argv, root);
    }

    /// internal
    private void checkNotifyRegisteredMethod (Method m, String notifyName)
	throws RuntimeException
    {
	Class[] p = m.getParameterTypes();
	if (p.length > 1) {

	    String msg = "Function \""+ m.getName () + "\" has " + p.length
		+ " arguments. You can't register a function with more than "
		+ "1 arguments with " + notifyName + ".";
	    throw new RuntimeException(msg);
	}

	if (p.length == 1 && p[0] != UVar.class) {
	    String msg = "Parameter 1 of the function \""
		+ m.getName () + "\" is of type "+ p[0].getName ()
		+ ". You can only register functions with a parameter of"
		+ " type liburbi.main.UVar with " + notifyName + ".";
	    throw new RuntimeException(msg);
	}

	if (m.getReturnType() != int.class) {
	    String msg = "Return type of the function \""
		+ m.getName () + "\" is "+ m.getReturnType().getName ()
		+ ". You can only register functions with a return type of"
		+ " type int with " + notifyName + ".";
	    throw new RuntimeException(msg);
	}
    }


    /// internal
    private static String getMethodBytecodeSignature(Method m)
    {
	return getMethodBytecodeSignature (m.getParameterTypes(),
					   m.getReturnType());
    }

    /// internal
    private static String getMethodBytecodeSignature(Class[] parameterTypes,
						     Class returnType)
    {
	StringBuffer sb = new StringBuffer();
	sb.append("(");
	for (int i=0; i<parameterTypes.length; i++)
	    sb.append(Type.make(parameterTypes[i]).getSignature());
	sb.append(")");
	sb.append(Type.make(returnType).getSignature());
	return sb.toString();
    }

    /// internal
    protected Method findMethodFromName (Object obj, String method_name)
	throws RuntimeException
    {
	Class obj_cls = obj.getClass ();
	Method[] methods = obj_cls.getMethods();
	Method res = null;
	for (int i = 0; i < methods.length; ++i) {
	    if (methods[i].getName() == method_name) {
		if (res == null)
		    res = methods[i];
		else
		{
		    String msg = "There are several methods with name "
			+ method_name + " in UObject "
			+ obj_cls.getName ()
			+ ", please specify the arguments to avoid ambiguities";
		    throw new RuntimeException(msg);
		}
	    }
	}
	if (res == null) {
	    String msg = "Can't find method with name "
		+ method_name + " in object "
		+ obj_cls.getName ();
	    throw new RuntimeException(msg);
	}
	return res;
    }

    /// internal
    protected Class[] stringTypeToClassType (String[] typeArray)
	throws java.lang.ClassNotFoundException
    {
	Class[] params = new Class[typeArray.length];
	for (int i = 0; i < typeArray.length; ++i) {
	    params[i] = Class.forName (typeArray[i]);
	}
	return params;
    }


    private static LinkedList<UObjectStarter> starterList = new LinkedList<UObjectStarter>();
}

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

import java.lang.reflect.Method;
import java.lang.reflect.Constructor;

import urbi.generated.urbi;
import urbi.generated.URBIStarterJAVA;
import urbi.generated.UContextImpl;
import urbi.generated.UObject;

public class UObjectStarter extends URBIStarterJAVA
{
    public UObjectStarter (String name, Constructor ctor) {
	super (name);
	uobject_ctor = ctor;
	uobject_name = name;
    }

    public UObject instanciate(UContextImpl ctx, String n)
    {
	String rn = n;
	if (rn.isEmpty())
	    rn = uobject_name; //getName();
	urbi.setCurrentContext (ctx);
	UObject res = null;

	try
	{
	    Object[] ctor_params = { rn };
	    res = (UObject) uobject_ctor.newInstance(ctor_params);
	    ctx.instanciated(res);
	    res.setCloner(this);
	}
     	catch (java.lang.InstantiationException e)
     	{
	    throw new RuntimeException (e);
     	}
	catch (java.lang.IllegalAccessException e)
     	{
	    throw new RuntimeException (e);
     	}
	catch (java.lang.reflect.InvocationTargetException e)
     	{
	    throw new RuntimeException (e);
     	}
	return res;
    }

    private Constructor uobject_ctor;
    private String uobject_name;

}


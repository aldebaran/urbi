/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

package urbi;

import java.lang.reflect.Method;
import java.lang.reflect.Constructor;

import urbi.urbi;
import urbi.URBIStarterJAVA;
import urbi.UContextImpl;
import urbi.UObjectCPP;

public class UObjectStarter extends URBIStarterJAVA
{
    public UObjectStarter (String name, Constructor ctor) {
	super (name);
	uobject_ctor = ctor;
	uobject_name = name;
    }

    public UObjectCPP instanciate(UContextImpl ctx, String n)
    {
	String rn = n;
	if (("").equals(rn))
	    rn = uobject_name; //getName();
	urbi.setCurrentContext (ctx);
	UObjectCPP res = null;

	try
	{
	    Object[] ctor_params = { rn };
	    res = (UObjectCPP) uobject_ctor.newInstance(ctor_params);
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


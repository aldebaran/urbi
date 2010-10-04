/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
package tests.java;

import urbi.UObject;


public class Java extends UObject
{
    static {
	UStart(Java.class);
    }

    public Java (String s) {
	super(s);
	UBindFunction("byte_");
	UBindFunction("short_");
	UBindFunction("int_");
	UBindFunction("long_");
    }

    public byte byte_(byte b)
    {
	System.out.println(b);
	return b;
    }

    public short short_(short b)
    {
	System.out.println(b);
	return b;
    }


    public int int_(int b)
    {
	System.out.println(b);
	return b;
    }


    public long long_(long b)
    {
	System.out.println(b);
	return b;
    }


}

/*! \file URBITalkie.java
*******************************************************************************

File: URBITalkie.java
Implementation of the URBITalkie class.

This file is part of
liburbi
(c) Bastien Saltel, 2004.

Permission to use, copy, modify, and redistribute this software for
non-commercial use is hereby granted.

This software is provided "as is" without warranty of any kind,
either expressed or implied, including but not limited to the
implied warranties of fitness for a particular purpose.

For more information, comments, bug reports: http://urbi.sourceforge.net

**************************************************************************** */

package examples.urbitalkie;

import java.io.IOException;

import liburbi.main.*;

import java.io.*;

public class	URBITalkie
{

    static {
	System.loadLibrary("urbijava");
    }

    public static ByteArrayOutputStream	out1;
    public static ByteArrayOutputStream	out2;
    public static UClient	r1;
    public static UClient	r2;

    public static void main(String[] args)
    {
	if (args.length < 2)
	{
	    System.err.println("Usage: urbitalkie robot1 robot2");
	    System.err.println("\tplays what robot1 hears with robot2's speaker, and vice-versa.");
	    System.exit(1);
	}
	r1 = new UClient(args[0]);
	r2 = new UClient(args[1]);
	out1 = new ByteArrayOutputStream();
	out2 = new ByteArrayOutputStream();

	MicroNotify1	mic1 = new MicroNotify1();
	MicroNotify2	mic2 = new MicroNotify2();

	r1.setCallback(mic1, "mic1");
	r2.setCallback(mic2, "mic2");

	r1.send("loop mic1<<micro.val,");
	r2.send("loop mic2<<micro.val,");

	liburbi.execute ();
    }
}

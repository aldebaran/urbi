/*! \file URBIMirror.java
*******************************************************************************

File: URBIMirror.java
Implementation of the URBIMirror class.

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

package examples.urbimirror;

import java.io.IOException;

import liburbi.main.*;

public class	URBIMirror
{
    static {
	System.loadLibrary("urbijava");
    }

    static public UClient	c = null;
    static public UClient	d = null;
    static public String devices[];
    static public int		devCount = 18;

    static public Command	cmd;

    public static void main(String[] args)
    {
	int		motorstate = 0;
	devices = new String[devCount];
	devices[0] = "legLF1";
	devices[1] = "legLF2";
	devices[2] = "legLF3";
	devices[3] = "legLH1";
	devices[4] = "legLH2";
	devices[5] = "legLH3";
	devices[6] = "legRH1";
	devices[7] = "legRH2";
	devices[8] = "legRH3";
	devices[9] = "legRF1";
	devices[10] = "legRF2";
	devices[11] = "legRF3";
	devices[12] = "neck";
	devices[13] = "headPan";
	devices[14] = "headTilt";
	devices[15] = "tailTilt";
	devices[16] = "tailPan";
	devices[17] = "mouth";

	if (args.length < 2)
	{
	    System.err.println("Usage: urbimirror sourcerobot destinationrobot [motorstate]");
	    System.err.println("Mirror the movements of one robot to the other.");
	    System.exit(1);
	}
	c = new UClient(args[0]);
	d = new UClient(args[1]);
	if (args.length >= 3)
	    motorstate = Integer.valueOf(args[2]).intValue();

	if (motorstate == 0)
	    c.send("motoroff;");
	d.send("motoron;");
	c.send("looptag<< loop {");
	cmd = new Command();
	for (int i = 0; i < devCount - 1; i++)
	{
	    c.setCallback(cmd, devices[i]);
	    c.send(devices[i] + "<< " + devices[i] + ".val&");
	}
	c.setCallback(cmd, devices[devCount - 1]);
	c.send(devices[devCount - 1] + "<< " + devices[devCount - 1] + ".val},");

	System.err.println("starting, hit ctrl-c to stop...");
	liburbi.execute ();
    }
}

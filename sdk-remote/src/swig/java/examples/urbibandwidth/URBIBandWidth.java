/*! \file URBIBandWidth.java
*******************************************************************************

File: URBIBandWidth.java
Implementation of the URBIBandWidth class.

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

package examples.urbibandwidth;

import java.io.IOException;

import urbi.*;
import urbi.*;

public class URBIBandWidth
{
     static {
	 System.loadLibrary("urbijava");
     }

    static public boolean	over = false;
    static public long		starttime;

    public static void main(String[] args)
    {
	if (args.length < 1)
	{
	    System.err.println("Usage: urbibandwidth robot");
	    System.exit(1);
	}

	UClient c = new UClient(args[0]);
	System.out.println("requesting raw images from server to test bandwidth...");
	BW		bw = new BW();
	c.setCallback(bw, "bw");
	BW		be = new BW();
	c.setCallback(be, "be");

	c.send ("camera.format = 0;camera.resolution = 0;noop;noop;");
	starttime = c.getCurrentTime ();
	c.send (" for (i=0;i<9; i++) bw << camera.val; be << camera.val;");
	urbi.execute ();
    }
}

/*! \file URBIImage.java
*******************************************************************************

File: URBIImage.java
Implementation of the URBIImage class.

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

    package examples.urbiimage;

import java.io.IOException;

import urbi.*;
import urbi.generated.*;

public class	URBIImage
{
    static {
	System.loadLibrary("urbijava");
    }

    static public UClient	robotC = null;

    public static void main(String[] args)
    {
	if (args.length != 1)
	{
	    System.err.println("Usage: urbiimage robot");
	    System.exit(1);
	}

	ImageSampler	imageSampler = new ImageSampler();

	robotC = new UClient(args[0]);
	try
	{
	    Thread.sleep(500);
	}
	catch (InterruptedException e)
	{
	    System.exit(1);
	}

	robotC.send("motoron;");
	robotC.send("camera.resolution = 0;");
	robotC.send("camera.format = 1;");
	robotC.send("camera.jpegfactor = 90;");

	ShowImage		image = new ShowImage(imageSampler);
	robotC.setCallback(image, "cam");

	robotC.send("loop cam<<camera.val, ");

	urbi.execute ();
    }
}

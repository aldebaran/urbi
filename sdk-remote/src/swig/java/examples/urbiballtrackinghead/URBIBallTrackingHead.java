/*! \file URBIBallTrackingHead.java
*******************************************************************************

File: URBIBallTrackingHead.java
Implementation of the URBIBallTrackingHead class.

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

    package examples.urbiballtrackinghead;

import java.io.IOException;

import urbi.UClient;
import urbi.ImageSampler;

public class	URBIBallTrackingHead
{

    static {
	System.loadLibrary("urbijava");
    }

    static public UClient	robotC = null;

    static public String	host = null;

    static public final	int	BALL_THRESHOLD  = 50;
    static public final	double	COMMAND_FACTOR_X  = 56.9;
    static public final	double	COMMAND_FACTOR_Y  = 45.2;
    static public final	double	MAXCOMMAND_X  = 150;
    static public final	double	MAXCOMMAND_Y  = 150;

    static public	double target_x = 0;
    static public	double target_y = 0;
    static public	double factor_x = COMMAND_FACTOR_X;
    static public	double factor_y = COMMAND_FACTOR_Y;

    static public int		framenum = 0;

    public URBIBallTrackingHead(String robotname)
    {
	ImageSampler	imageSampler = new ImageSampler();

	robotC = new UClient(robotname);
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
	robotC.send("camera.jpegfactor = 75;");

	CallImage		image = new CallImage(imageSampler);
	robotC.setCallback(image, "cam");

	robotC.send("loop cam<<camera.val, ");

	try
	{
	    Thread.sleep(200000);
	}
	catch (InterruptedException e)
	{
	    System.exit(0);
	}
    }

    public static void main(String[] args)
    {
	if (args.length != 1)
	{
	    System.err.println("Usage: urbiballtrackinghead robot");
	    System.exit(1);
	}
	URBIBallTrackingHead bt = new URBIBallTrackingHead(args[0]);
	while (true) {}
    }
}

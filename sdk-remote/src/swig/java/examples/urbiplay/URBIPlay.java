/*! \file URBIPlay.java
*******************************************************************************

File: URBIPlay.java
Implementation of the URBIPlay class.

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

package examples.urbiplay;

import java.io.IOException;

import java.io.*;
import liburbi.main.*;

public class	URBIPlay
{
    static {
	System.loadLibrary("urbijava");
    }

    static public char			dumpMode;
    static public int			devCount = 0;
    static public BufferedInputStream	in;

    static public String devices[];

    static public UClient	robot = null;

    private static int		convertByteToInt(byte[] b)
    {
	int		res = 0;

	for (int i = b.length - 1; i >= 0; i--)
	{
	    if (i == 3)
		res += b[i] * Math.pow(256, i);
	    else
		res += (b[i] & 0xff) * Math.pow(256, i);
	}
	return res;
    }

    private static short	convertByteToShort(byte[] b)
    {
	short		res = 0;

	for (int i = b.length - 1; i >= 0; i--)
	{
	    if (i == 1)
		res += b[i] * Math.pow(256, i);
	    else
		res += (b[i] & 0xff) * Math.pow(256, i);
	}
	return res;
    }

    private static boolean	parseHeader()
    {
	byte[]	buff1 = new byte[4];
	byte[]	buff2 = new byte[4];

	try
	{
	    in.read(buff1, 0, 4);
	    String tmp1 = new String(buff1);
	    if (tmp1.equals("URBI") == false)
		return false;
	    in.read(buff2, 0, 4);
	    devCount = convertByteToInt(buff2);
	    devices = new String[devCount];
	    for (int i = 0;i < devCount; i++)
	    {
		byte[]	device = new byte[256];
		byte[]	tmp2 = new byte[2];
		byte[]	tmp3 = new byte[4];
		String	name;
		short	id;
		int		type;
		int off = 0;

		do
		{
		    in.read(device, off, 1);
		    off++;
		}
		while (device[off - 1] != 0);
		name = new String(device, 0, off - 1);
		devices[i] = name;

		in.read(tmp2, 0, 2);
		id = convertByteToShort(tmp2);

		in.read(tmp3, 0, 1);
		type = (int)tmp3[0];
	    }
	}
	catch (IOException e)
	{
	    return false;
	}
	return true;
    }

    private static void	play()
    {
	int		timestamp;
	long	prevtime = 0;
	long	starttime = 0;
	long	ttime = 0;
	long	sleepstop = 0;

	long	tick = 0;
	short	id;
	float	angle;
	boolean	pending = false;
	try
	{
	    while (in.available() > 0)
	    {
		int		nb;
		byte[]	tmp1 = new byte[4];
		byte[]	tmp2 = new byte[2];
		byte[]	tmp3 = new byte[8];
		byte[]	tmp4 = new byte[2];
		int		tmp5;
		String	device;

		in.read(tmp1, 0, 4);
		timestamp = convertByteToInt(tmp1);
		if (starttime == 0)
		    starttime = System.currentTimeMillis() - timestamp - 1;
		sleepstop = timestamp + starttime;
		in.read(tmp2, 0, 2);
		id = convertByteToShort(tmp2);
		if (id > devCount)
		{
		    System.err.println("device id " + id + " not found");
		    continue ;
		}
		device = devices[id];

		in.read(tmp4, 0, 2);
		in.read(tmp3, 0, 4);
		tmp5 = convertByteToInt(tmp3);
		angle = Float.intBitsToFloat(tmp5);

		if (robot != null)
		{
		    if (sleepstop > System.currentTimeMillis())
		    {
			if (pending == true)
			{
			    robot.send("noop;");
			    pending = false;
			}
			try
			{
			    Thread.sleep(sleepstop - System.currentTimeMillis());
			}
			catch (InterruptedException e)
			{
			}
		    }
		    if (pending == false)
			pending = true;
		    robot.send(device + ".val = " + angle + "&");
		}
		else
		{
		    if (dumpMode == '-')
			System.out.println("[" + timestamp + "] " + device +  ".val = " + angle);
		    else
		    {
			if ((timestamp != prevtime) && (prevtime != 0))
			    System.out.println("noop;");
			prevtime = timestamp;
			System.out.println(device + ".val = " + angle + "&");
		    }
		}
		if ((tick % 1000) == 0)
		{
		    if (tick != 0)
		    {
			float tmp = 1000000 / (System.currentTimeMillis() - ttime);
			System.err.println(tmp + " cps");
		    }
		    ttime = System.currentTimeMillis();
		}
		tick++;
		tmp1 = null;
		tmp2 = null;
		tmp3 = null;
		tmp4 = null;
	    }
	}
	catch (IOException e)
	{
	    System.err.println("Error while reading");
	}
	if ((robot != null) && (pending == true))
	{
	    robot.send("noop;");
	    pending = false;
	}
	if ((robot == null) && (dumpMode != '-'))
	    System.out.println("noop;");
    }

    public static void main(String[] args)
    {
	if (args.length < 2)
	{
	    System.err.println("usage: urbiplay robot file [loop] ");
	    System.err.println("\t Pass '-' as 'robotname' to dump to stdout in human-readable format,");
	    System.err.println("\t  or '+' to dump to stdout in urbi format.");
	    System.exit(1);
	}
	int loop = 0;
	if (args.length >= 3)
	    loop = Integer.valueOf(args[2]).intValue();

	if ((args[0].equals("-") == true) || (args[0].equals("+") == true))
	{
	    robot = null;
	    dumpMode = args[0].charAt(0);
	}
	else
	{
	    robot = new UClient(args[0]);
	    robot.send("motoron;");
	}
	try
	{
	    in = new BufferedInputStream(new FileInputStream(args[1]));
	}
	catch (FileNotFoundException e)
	{
	    System.err.println("error opening file");
	    System.exit(2);
	}
	if (parseHeader() == false)
	{
	    System.err.println("error parsing header");
	    System.exit(3);
	}
	play();
	try
	{
	    in.close();
	}
	catch (IOException e)
	{
	}
	liburbi.execute ();
    }
}

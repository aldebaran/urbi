/*! \file URBIPing.java
*******************************************************************************

File: URBIPing.java
Implementation of the URBIPing class.

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

package examples.urbiping;

import java.io.IOException;

import liburbi.main.UClient;

public class URBIPing
{
    static {
	System.loadLibrary("urbijava");
    }

    static public UClient	c = null;

    static public String	rname = null;

    static public long		count;

    static public long		pingCount = 0;

    static public boolean	received;

    static public boolean	over = false;

    static public long		sendtime;

    static public long		avgtime = 0;

    static public long		mintime = 0;

    static public long		maxtime = 0;

    public static void		showstats()
    {
	if (pingCount == 0)
	    System.exit(1);
	long		i = avgtime / pingCount;

	System.out.println("rtt min/avg/max " + mintime + " " +  i + " " + maxtime);
	System.exit(1);
    }

    public static void main(String[] args)
    {
	if (args.length < 1)
	{
	    System.err.println("Usage: urbiping robot [msinterval] [count]");
	    System.exit(1);
	}
	rname = args[0];
	c = new UClient(rname);
	count = 0;
	long interval = 1000;
	if (args.length > 1)
	    interval = Long.valueOf(args[1]).longValue();
	if (args.length > 2)
	    count = Long.valueOf(args[2]).longValue();
	UPing		pong = new UPing();
	c.setCallback(pong, "uping");
	try
	{
	    Thread.sleep(1000);
	}
	catch (InterruptedException e)
	{
	}
	received = true;
	for (int i = 0; i < count || count == 0; i++)
	{
	    while (!received)
	    {
		try
		{
		    Thread.sleep(1000);
		}
		catch (InterruptedException e)
		{
		}
	    }
	    received = false;
	    sendtime = System.currentTimeMillis();

	    c.send("uping<<ping;");
	    try
	    {
		Thread.sleep(interval);
	    }
	    catch (InterruptedException e)
	    {
	    }

	}
	while (over == false)
	{
	    try
	    {
		Thread.sleep(1000);
	    }
	    catch (InterruptedException e)
	    {
	    }
	}
	showstats();
    }
}

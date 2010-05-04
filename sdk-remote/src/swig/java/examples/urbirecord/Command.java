/*! \file Command.java
*******************************************************************************

File: Command.java
Implementation of the Command class.

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

package examples.urbirecord;

import java.io.IOException;

import liburbi.main.*;

public class	Command extends UCallbackInterface
{
    public long		tme = 0;
    public long		tilt = 0;

    public Command()
    {
	super ();
    }

    public UCallbackAction   	onMessage(UMessage msg)
    {
	for (int i = 0; i < URBIRecord.devCount; i++)
	{
	    if (msg.getTag().equals(URBIRecord.devices[i]) == true)
	    {
		URBIRecord.out.write(URBIRecord.convertIntToByte(msg.getTimestamp()), 0, 4);
		URBIRecord.out.write(URBIRecord.convertShortToByte((short)i), 0, 2);
		URBIRecord.out.write((byte)0);
		URBIRecord.out.write((byte)0);
		URBIRecord.out.write(URBIRecord.convertIntToByte(Float.floatToIntBits((float) (msg.getValue ().getDouble ()))), 0, 4);
		tilt++;
		if ((tilt % 100) == 0)
		{
		    if (tme != 0)
		    {
			float tmp = 1000000 / (System.currentTimeMillis() - tme);
			System.err.println(tmp + " cps");
		    }
		    tme = System.currentTimeMillis();
		}
		return UCallbackAction.URBI_CONTINUE;
	    }
	}
	System.err.println("Error: no device " + msg.getTag() + " (in " + msg.getValue ().getDouble () + ")");
	return UCallbackAction.URBI_CONTINUE;
    }
}

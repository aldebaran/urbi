/*! \file UPing.java
 *******************************************************************************

 File: UPing.java
 Implementation of the UPing class.

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

import liburbi.main.*;

public class	UPing extends UCallbackInterface
{
	public UPing()
	{
	    super ();
	}

	public UCallbackAction onMessage(UMessage msg)
	{
		long	tv = System.currentTimeMillis();
		long	ptime = tv - URBIPing.sendtime;

		if ((URBIPing.pingCount == 0) || URBIPing.mintime > ptime)
			URBIPing.mintime = ptime;
		if ((URBIPing.pingCount == 0) || URBIPing.maxtime < ptime)
			URBIPing.maxtime = ptime;
		URBIPing.avgtime += ptime;
		URBIPing.pingCount++;
		System.out.println("ping reply from " + URBIPing.rname + ": seq="
						   + URBIPing.pingCount + " time=" + ptime + " ms");
		URBIPing.received = true;
		if (URBIPing.pingCount == URBIPing.count)
			URBIPing.over = true;
		return UCallbackAction.URBI_CONTINUE;
	}
}

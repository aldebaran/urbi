/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

package examples.urbiping;

import java.io.IOException;

import urbi.*;
import urbi.*;

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

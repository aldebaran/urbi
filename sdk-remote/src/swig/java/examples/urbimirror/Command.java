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

package examples.urbimirror;

import java.io.IOException;

import urbi.*;
import urbi.*;

public class Command extends UCallbackInterface
{
    public Command()
    {
	super ();
    }

    public UCallbackAction onMessage (UMessage msg)
    {
	if (!(UMessageType.MESSAGE_DATA == msg.getType ()) ||
	    !(UDataType.DATA_DOUBLE == msg.getValue ().getType ()))
	    return UCallbackAction.URBI_CONTINUE;

	double val = msg.getValue ().doubleValue ();
	URBIMirror.d.send (msg.getTag() + ".val = " + val + ";");
	return UCallbackAction.URBI_CONTINUE;
    }
}

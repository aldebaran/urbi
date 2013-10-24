/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

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

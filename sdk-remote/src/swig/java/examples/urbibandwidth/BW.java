/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

package examples.urbibandwidth;

import java.io.IOException;

import urbi.*;
import urbi.*;

public class BW extends UCallbackInterface
{
    static private long totalsize = 0;

    public BW()
    {
        super ();
    }

    public UCallbackAction onMessage(UMessage msg)
    {
        if (!(UMessageType.MESSAGE_DATA == msg.getType ()))
        {
            System.err.println ("Error: received non data message.");
            return UCallbackAction.URBI_CONTINUE;
        }
        UValue value = msg.getValue ();
        if (!(UDataType.DATA_BINARY == value.getType ()))
        {
            System.err.println ("Error: received non binary message.");
            return UCallbackAction.URBI_CONTINUE;
        }
        UBinary bin = value.ubinaryValue ();
        if (!(UBinaryType.BINARY_IMAGE == bin.getType ()))
        {
            System.err.println ("Error: received non image message.");
            return UCallbackAction.URBI_CONTINUE;
        }
        UImage img = bin.uimageValue ();

        totalsize += (img.getSize () + msg.getTag ().length () + 20);

        if (msg.getTag() != "be")
        {
            long    end = msg.getClient ().getCurrentTime ();
            long    tmp = end - URBIBandWidth.starttime;
            long    tmp2 = totalsize * 1000 / (end - URBIBandWidth.starttime);

            System.out.println("received " + totalsize + " bytes in " +
                               tmp + " milliseconds: bandwidth is " +
                               tmp2 + " bytes per second.");
            //URBIBandWidth.over = true;
        }
        return UCallbackAction.URBI_CONTINUE;
    }
}

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

public class URBIBandWidth
{
    static {
        System.loadLibrary("urbijava");
    }

    static public boolean   over = false;
    static public long      starttime;

    public static void main(String[] args)
    {
        if (args.length < 1)
        {
            System.err.println("Usage: urbibandwidth robot");
            System.exit(1);
        }

        UClient c = new UClient(args[0]);
        System.out.println("requesting raw images from server to test bandwidth...");
        BW      bw = new BW();
        c.setCallback(bw, "bw");
        BW      be = new BW();
        c.setCallback(be, "be");

        c.send ("camera.format = 0;camera.resolution = 0;noop;noop;");
        starttime = c.getCurrentTime ();
        c.send (" for (i=0;i<9; i++) bw << camera.val; be << camera.val;");
        urbi.execute ();
    }
}

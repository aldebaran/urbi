/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

package examples.urbiimage;

import java.io.IOException;

import urbi.*;
import urbi.*;

public class    URBIImage
{
    static {
        System.loadLibrary("urbijava");
    }

    static public UClient   robotC = null;

    public static void main(String[] args)
    {
        if (args.length != 1)
        {
            System.err.println("Usage: urbiimage robot");
            System.exit(1);
        }

        ImageSampler    imageSampler = new ImageSampler();

        robotC = new UClient(args[0]);
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
        robotC.send("camera.jpegfactor = 90;");

        ShowImage       image = new ShowImage(imageSampler);
        robotC.setCallback(image, "cam");

        robotC.send("loop cam<<camera.val, ");

        urbi.execute ();
    }
}

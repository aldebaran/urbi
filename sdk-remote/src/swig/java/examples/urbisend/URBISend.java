/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

package examples.urbisend;

import java.io.IOException;

import urbi.*;
import urbi.*;

public class	URBISend
{
    static {
	 System.loadLibrary("urbijava");
     }

    public static void main(String[] args)
    {
	if (args.length < 2)
	{
	    System.err.println("Missing file name");
	    System.err.println("Usage: urbisend robot filename");
	    System.exit(1);
	}
	UClient	client =  new UClient(args[0]);

	client.sendFile(args[1]);
	System.out.println("File sent, hit Ctrl-C to terminate.");
	urbi.execute ();

    }
}

/*! \file URBISend.java
*******************************************************************************

File: URBISend.java
Implementation of the URBISend class.

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

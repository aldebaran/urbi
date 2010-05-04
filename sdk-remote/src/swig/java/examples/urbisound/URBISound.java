/*! \file URBISound.java
*******************************************************************************

File: URBISound.java
Implementation of the URBISound class.

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

package examples.urbisound;

import java.io.IOException;

import	liburbi.main.*;
import	liburbi.sound.*;

public class	URBISound
{
    static {
	System.loadLibrary("urbijava");
    }

    static public UClient	robotC = null;

    static public SoundSampler		soundSampler = null;

    public static void main(String[] args)
    {
	if (args.length != 1)
	{
	    System.err.println("Usage: urbisound robot");
	    System.exit(1);
	}
	robotC = new UClient(args[0]);

	CallSound	sound = new CallSound();
	robotC.setCallback(sound, "sound");

	robotC.send("sound<< ping;");
	soundSampler = new SoundSampler();
	soundSampler.setAction(sound);

	liburbi.execute ();
    }
}

/*! \file MicroNotify2.java
*******************************************************************************

File: MicroNotify2.java
Implementation of the MicroNotify2 class.

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

package examples.urbitalkie;

import java.io.IOException;

import urbi.*;
import urbi.generated.*;

import java.io.*;

public class MicroNotify2 extends UCallbackInterface
{
    public MicroNotify2()
    {
	super ();
    }

    public UCallbackAction     	onMessage(UMessage msg)
    {
	if (!(UMessageType.MESSAGE_DATA == msg.getType ()))
	    return UCallbackAction.URBI_CONTINUE;
	UValue value = msg.getValue ();

	if (!(UDataType.DATA_BINARY == value.getType ()))
	    return UCallbackAction.URBI_CONTINUE;
	UBinary bin = value.getUBinary ();

	if (!(UBinaryType.BINARY_SOUND == bin.getType ()))
	    return UCallbackAction.URBI_CONTINUE;
	USound sound = bin.getUSound ();
	byte[] data = sound.getDataAsByte ();

	synchronized (URBITalkie.out2)
	{
	    if (URBITalkie.out2.size() < 10000)
	    {
		System.out.println("MicroNotify2 : write");
		URBITalkie.out2.write(data, 0, data.length);
	    }
	}
	synchronized (URBITalkie.out1)
	{
	    if (URBITalkie.out1.size() > 0)
	    {
		System.out.println("MicroNotify2 : begin send size : " + URBITalkie.out1.size());
		byte audio[] = URBITalkie.out1.toByteArray();

		System.out.println("MicroNotify2 : begin send");
		URBITalkie.r2.sendBin(audio, audio.length,
				      "speaker.val = BIN " + audio.length + " raw "
				      + sound.getChannels() + " " + (int)sound.getRate() +
				      " " + sound.getSampleSize() + " 1;");

		System.out.println("MicroNotify2 : end send");
		audio = null;
		URBITalkie.out1.reset();
		System.out.println("MicroNotify2 : end send");
	    }
	}
	return UCallbackAction.URBI_CONTINUE;
    }
}

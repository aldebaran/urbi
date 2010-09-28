/*! \file CallSound.java
*******************************************************************************

File: CallSound.java
Implementation of the CallSound class.

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

import java.io.*;

import urbi.*;
import urbi.*;
import javax.sound.sampled.*;

public class CallSound extends UCallbackInterface implements SoundAction
{
    private	ByteArrayOutputStream		out;

    private static float	sampleRate = 8000;

    private	static int		sampleSizeInBits = 8;

    private	static int		channels = 1;

    private static boolean	signed = true;

    private boolean	isCaptured = false;

    public CallSound()
    {
	super ();
	out = new ByteArrayOutputStream();
    }

    public UCallbackAction onMessage(UMessage msg)
    {
	if (!(UMessageType.MESSAGE_DATA == msg.getType ()))
	    return UCallbackAction.URBI_CONTINUE;
	UValue value = msg.getValue ();

	if (!(UDataType.DATA_BINARY == msg.getValue ().getType ()))
	    return UCallbackAction.URBI_CONTINUE;
	UBinary bin = value.getUBinary ();

	if (!(UBinaryType.BINARY_SOUND == bin.getType ()))
	    return UCallbackAction.URBI_CONTINUE;
	USound sound = bin.getUSound();

	sampleRate = (int) sound.getRate ();
	sampleSizeInBits = (int) sound.getSampleSize ();
	channels = (int) sound.getChannels ();
	signed = (sound.getSampleFormat () == USoundSampleFormat.SAMPLE_SIGNED);
	byte[] data = sound.getData ();
	out.write(data, 0, data.length);
	return UCallbackAction.URBI_CONTINUE;
    }

    public void		captureAudio()
    {
	if (isCaptured == false)
	{
	    out.reset();
	    isCaptured = true;
	}
	URBISound.robotC.send("loopsound<<loop sound<<micro.val, ");
    }

    public void		playAudio()
    {
	try
	{
	    byte audio[] = out.toByteArray();
	    InputStream input = new ByteArrayInputStream(audio);

	    SoundUtilities.setSampleRate(sampleRate);
	    SoundUtilities.setSampleSizeInBits(sampleSizeInBits);
	    SoundUtilities.setChannels(channels);
	    SoundUtilities.setSigned(signed);
	    final AudioFormat format = SoundUtilities.getFormat();

	    final AudioInputStream ais = new AudioInputStream(input, format,
							      audio.length / format.getFrameSize());
	    SourceDataLine.Info info = new DataLine.Info(SourceDataLine.class, ais.getFormat(),
							 ((int)ais.getFrameLength() * format.getFrameSize()));
	    final SourceDataLine line = (SourceDataLine)AudioSystem.getLine(info);
	    line.open(ais.getFormat());
	    line.start();

	    try
	    {
		int		numRead = 0;
		byte[]	buf = new byte[line.getBufferSize()];
		while ((numRead = ais.read(buf, 0, buf.length)) >= 0)
		{
		    int offset = 0;
		    while (offset < numRead)
		    {
			offset += line.write(buf, offset, numRead - offset);
		    }
		}
		line.drain();
		line.close();
		input.close();
	    }
	    catch (IOException e)
	    {
		System.err.println("I/O problems: " + e);
		System.exit(1);
	    }
	    audio = null;
	}
	catch (LineUnavailableException e)
	{
	    System.err.println("Line unavailable: " + e);
	    System.exit(1);
	}
    }

    public void		stopAudio()
    {
	try
	{
	    URBISound.robotC.send(" stop loopsound; wait(1000); ");
	    out.close();
	    isCaptured = false;
	}
	catch (IOException e)
	{
	}
    }

    public void		saveAudio(String filename)
    {
	File	file = new File(filename);

	try
	{
	    if (file.exists() == true)
		file.delete();
	    file.createNewFile();

	    byte audio[] = out.toByteArray();
	    InputStream input = new ByteArrayInputStream(audio);

	    SoundUtilities.setSampleRate(sampleRate);
	    SoundUtilities.setSampleSizeInBits(sampleSizeInBits);
	    SoundUtilities.setChannels(channels);
	    SoundUtilities.setSigned(signed);
	    final AudioFormat format = SoundUtilities.getFormat();

	    final AudioInputStream ais = new AudioInputStream(input, format,
							      audio.length / format.getFrameSize());
	    AudioSystem.write(ais, AudioFileFormat.Type.WAVE, file);
	}
	catch (IOException e)
	{
	}
    }
}

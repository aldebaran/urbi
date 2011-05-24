/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

    package urbi;

import javax.sound.sampled.*;
import urbi.UClient;

import java.io.*;

/**
 * The SoundUtilities class is full of handy static methods.
 * <p>
 * <p>
 * @author Bastien Saltel
 */

public class	SoundUtilities
{
    private static float	sampleRate = 8000;

    private	static int		sampleSizeInBits = 8;

    private	static int		channels = 1;

    private static boolean	signed = true;

    private static boolean	bigEndian = false;

    /**
     * Sets the sample rate.
     * <p>
     */
    public static void		setSampleRate(float f)
    {
	sampleRate = f;
    }

    /**
     * Returns the sample rate.
     * <p>
     */
    public static float	getSampleRate()
    {
	return sampleRate;
    }

    /**
     * Sets the sample size.
     * <p>
     */
    public static void		setSampleSizeInBits(int i)
    {
	sampleSizeInBits = i;
    }

    /**
     * Returns the sample size.
     * <p>
     */
    public static int		getSampleSizeInBits()
    {
	return sampleSizeInBits;
    }

    /**
     * Sets the number of channels.
     * <p>
     */
    public static void		setChannels(int i)
    {
	channels = i;
    }

    /**
     * Returns the number of channels.
     * <p>
     */
    public static int		getChannels()
    {
	return channels;
    }

    /**
     * Sets the signed value.
     * <p>
     */
    public static void		setSigned(boolean i)
    {
	signed = i;
    }

    /**
     * Returns true if audio data is signed.
     * <p>
     * @return True if audio data is signed,
     *         false otherwise.
     */
    public static boolean	getSigned()
    {
	return signed;
    }

    /**
     * Sets the big endian value.
     * <p>
     */
    public static void		setBigEndian(boolean i)
    {
	bigEndian = i;
    }

    /**
     * Returns true if audio data is in big endian mode.
     * <p>
     * @return True if audio data is in big endian mode,
     *         false otherwise.
     */
    public static boolean	getBigEndian()
    {
	return bigEndian;
    }


    /*
     * Returns the data audio format.
     * <p>
     */
    public static AudioFormat	getFormat()
    {
	return new AudioFormat(sampleRate,
			       sampleSizeInBits,
			       channels,
			       signed,
			       bigEndian);
    }

    /*
     * Sends an audio file to the remote host which the client is connected to.
     * <p>
     * @param file  The name of file.
     * @param client A reference to the client.
     */
    public static int	sendSound(String filename, UClient client) throws IOException
    {
	int		totalFramesRead = 0;
	File	file = new File(filename);

	try
	{
	    AudioFileFormat fformat = AudioSystem.getAudioFileFormat(file);
	    AudioFormat		format = fformat.getFormat();
	    try
	    {
		AudioInputStream	ais = AudioSystem.getAudioInputStream(file);
		int numBytes = 2048;
		byte[] audioBytes = new byte[numBytes];

		int numBytesRead = 0;
		try
		{
		    while ((numBytesRead = ais.read(audioBytes)) != -1)
		    {
			totalFramesRead += numBytesRead;
// 			client.send("speaker.val = BIN " + numBytesRead + " raw "
// 				    + format.getChannels() + " " + (int)format.getSampleRate() +
// 				    " " + format.getSampleSizeInBits() + " 1;");
			client.sendBin (audioBytes, numBytesRead,
					"speaker.val = BIN " + numBytesRead + " raw "
					+ format.getChannels() + " " + (int)format.getSampleRate() +
					" " + format.getSampleSizeInBits() + " 1;");
		    }
		}
		catch (IOException e)
		{
		    System.out.println("Read error");
		}
	    }
	    catch (UnsupportedAudioFileException e)
	    {
	    }
	}
	catch (UnsupportedAudioFileException e)
	{
	}
	return 0;
    }
}

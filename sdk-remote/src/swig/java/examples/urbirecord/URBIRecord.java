/*! \file URBIRecord.java
*******************************************************************************

File: URBIRecord.java
Implementation of the URBIRecord class.

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

package examples.urbirecord;

import java.io.IOException;
import java.io.*;

import liburbi.main.*;

/*
  file format
  URBI (4 bytes)
  numjoints (4 bytes)
  [numjoint times] namejoint (char * nullterminated) id (short)  type(char)
  [til end of file]
  timestamp [int, milisec] id (short) value (UValue)
*/

public class	URBIRecord
{
    static {
	System.loadLibrary("urbijava");
    }

    static public UClient	c = null;
    static public PrintStream	out = null;

    static public String devices[];
    static public int		devCount = 18;

    static public Command	cmd;

    static public byte[]	convertIntToByte(int nb)
    {
	byte[] b = new byte[4];

	for (int i = 0; i < 4; i++)
	{
	    b[i] = (byte)(nb >> (i * 8));
	}
	return b;
    }

    static public byte[]	convertShortToByte(int nb)
    {
	byte[] b = new byte[2];

	for (int i = 0; i < 2; i++)
	{
	    b[i] = (byte)(nb >> (i * 8));
	}
	return b;
    }

    private static void		buildHeader()
    {
	out.print("URBI");
	out.write(convertIntToByte(devCount), 0, 4);
	for (int i = 0; i < devCount; i++)
	{
	    out.print(devices[i]);
	    out.write((byte)0);
	    out.write(convertShortToByte((short)i), 0, 2);
	    out.write((byte)1);
	}
	out.flush();
    }

    public static void main(String[] args)
    {
	devices = new String[devCount];
	devices[0] = "legLF1";
	devices[1] = "legLF2";
	devices[2] = "legLF3";
	devices[3] = "legLH1";
	devices[4] = "legLH2";
	devices[5] = "legLH3";
	devices[6] = "legRH1";
	devices[7] = "legRH2";
	devices[8] = "legRH3";
	devices[9] = "legRF1";
	devices[10] = "legRF2";
	devices[11] = "legRF3";
	devices[12] = "neck";
	devices[13] = "headPan";
	devices[14] = "headTilt";
	devices[15] = "tailTilt";
	devices[16] = "tailPan";
	devices[17] = "mouth";

	if (args.length < 2)
	{
	    System.err.println("usage: urbirecord robotname file");
	    System.err.println("\t Records a sequence of movements to a file.");
	    System.exit(1);
	}
	c = new UClient(args[0]);
	if (args[1].equals("-") == true)
	    out = System.out;
	else
	{
	    try
	    {
		File file = new File(args[1]);
		if (file.exists() == true)
		    file.delete();
		file.createNewFile();
		out = new PrintStream(new BufferedOutputStream(new FileOutputStream(file)));
	    }
	    catch (IOException e)
	    {
	    }
	}

	buildHeader();
	c.send("looptag<< loop {");
	cmd = new Command();
	for (int i = 0; i < devCount - 1; i++)
	{
	    c.setCallback(cmd, devices[i]);
	    c.send(devices[i] + "<< " + devices[i] + ".val&");
	}
	c.setCallback(cmd, devices[devCount - 1]);
	c.send(devices[devCount - 1] + "<< " + devices[devCount - 1] + ".val},");
	System.err.println("starting, hit ctrl-c to stop...");

	liburbi.execute ();
    }
}

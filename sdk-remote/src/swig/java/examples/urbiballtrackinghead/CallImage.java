/*! \file CallImage.java
*******************************************************************************

File: CallImage.java
Implementation of the CallImage class.

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

    package examples.urbiballtrackinghead;

import java.io.IOException;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;

import liburbi.main.*;
import liburbi.image.*;



import liburbi.main.UClient;

public class	CallImage extends UCallbackInterface
{
    private static ImageSampler	imageSampler;

    private MedianFilter		filter_x;
    private MedianFilter		filter_y;

    public CallImage(ImageSampler sampler)
    {
	super ();
	this.imageSampler = sampler;
	filter_x = new MedianFilter(5);
	filter_y = new MedianFilter(5);
    }

    public CallImage()
    {
	super ();
	filter_x = new MedianFilter(5);
	filter_y = new MedianFilter(5);
    }

    public void		doSendCommand()
    {
	filter_x.addElement(URBIBallTrackingHead.target_x);
	filter_y.addElement(URBIBallTrackingHead.target_y);

	System.out.println(filter_x.toString());
	if (filter_x.checkSize() == true)
	{
	    URBIBallTrackingHead.robotC.send("headPan.val = headPan.val + " + filter_x.getMedian() + ", headTilt.val = headTilt.val + " + filter_y.getMedian() + ",");
	}
    }


    public UCallbackAction onMessage(UMessage msg)
    {
	if (!(UMessageType.MESSAGE_DATA == msg.getType ()))
	    return UCallbackAction.URBI_CONTINUE;

	UValue value = msg.getValue ();

	if (!(UDataType.DATA_BINARY == value.getType ()))
	    return UCallbackAction.URBI_CONTINUE;

	UBinary bin = value.getUBinary ();

	if (!(UBinaryType.BINARY_IMAGE == bin.getType ()))
	    return UCallbackAction.URBI_CONTINUE;

	UImage img = bin.getUImage ();
	byte[] data = img.getDataAsByte ();

	int		nummatch = 0;
	int		width = (int) img.getWidth();
	int		height = (int) img.getHeight();
	int[]	pixels;

	BufferedImage buffer = null;
	try
	{
	    ByteArrayInputStream bais = new ByteArrayInputStream(data);
	    com.sun.image.codec.jpeg.JPEGImageDecoder decoder = com.sun.image.codec.jpeg.JPEGCodec.createJPEGDecoder(bais);
	    buffer = decoder.decodeAsBufferedImage();
	} catch (Exception e)
	{
	}

	double	red = 0;
	double	green = 0;
	double	blue = 0;
	double	sum_x = 0;
	double	sum_y = 0;
	double	ball_x = 0;
	double	ball_y = 0;
	double	dball_x = 0;
	double	dball_y = 0;


	pixels = buffer.getRGB(0, 0, width, height, null, 0, width);

	for (int i = 0; i < pixels.length; i++)
	{
	    red = ImageFilter.getRed(pixels[i]);
	    green = ImageFilter.getGreen(pixels[i]);
	    blue = ImageFilter.getBlue(pixels[i]);
	    if ((red >= 190) && (red <= 250)
		&& (green >= 15) && (green <= 70)
		&& (blue >= 70) && (blue <= 140))
	    {
		nummatch++;
		sum_y += i / width;
		sum_x += i % width;
	    }
	}
	pixels = null;
	if (nummatch >= URBIBallTrackingHead.BALL_THRESHOLD)
	{
	    ball_x = sum_x / nummatch;
	    ball_y = sum_y / nummatch;
	    dball_x = ball_x - (double)width / 2;
	    dball_y = ball_y - (double)height / 2;
	    URBIBallTrackingHead.target_x = (-1) * (URBIBallTrackingHead.factor_x / (double)width) * dball_x;
	    URBIBallTrackingHead.target_y = (-1) * (URBIBallTrackingHead.factor_y / (double)height) * dball_y;
	    if (URBIBallTrackingHead.target_x > 90.0)
		URBIBallTrackingHead.target_x = 90.0;
	    if (URBIBallTrackingHead.target_x < -90.0)
		URBIBallTrackingHead.target_x = -90.0;
	    if (URBIBallTrackingHead.target_y > 60.0)
		URBIBallTrackingHead.target_y = 60.0;
	    if (URBIBallTrackingHead.target_y < -30.0)
		URBIBallTrackingHead.target_y = -30.0;
	    doSendCommand();
	    imageSampler.addImage(buffer, (int) img.getWidth(), (int) img.getHeight(),
				  ball_x, ball_y);
	}
	else
	    imageSampler.addImage(buffer, (int) img.getWidth(), (int) img.getHeight());
	buffer.flush();
	buffer = null;

	return UCallbackAction.URBI_CONTINUE;
    }
}

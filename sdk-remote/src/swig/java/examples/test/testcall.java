
package examples.test;


import urbi.*;
import urbi.*;

import java.io.IOException;
import java.io.ByteArrayInputStream;
import java.awt.Image;
import java.awt.image.BufferedImage;


public class testcall extends UCallbackInterface {

    private static ImageSampler	imageSampler;

    public testcall() {
	super();
	imageSampler = new ImageSampler();
    }

    public UCallbackAction onMessage(UMessage msg) {

	System.out.println ("");
	if (UMessageType.MESSAGE_SYSTEM == msg.getType ())
	    System.out.println ("MESSAGE_SYSTEM msg: " + msg.getMessage ());
	else if (UMessageType.MESSAGE_ERROR == msg.getType ())
	    System.out.println ("MESSAGE_ERROR msg: " + msg.getMessage ());
	else if (UMessageType.MESSAGE_DATA == msg.getType ())
	{
	    System.out.println ("MESSAGE_DATA msg");
	    UValue value = msg.getValue ();
	    //System.out.println ("Received: " + value);

	    if (UDataType.DATA_DOUBLE == value.getType ())
		System.out.println (" DATA_DOUBLE value: " + value.getDouble ());
	    else if (UDataType.DATA_STRING == value.getType ())
		System.out.println (" DATA_STRING value: " + value.getString ());
	    else if (UDataType.DATA_BINARY == value.getType ())
	    {
		UBinary bin = value.getUBinary ();
		System.out.println (" DATA_BINARY value");
		if (UBinaryType.BINARY_NONE == bin.getType ())
		    System.out.println ("  BINARY_NONE binary");
		else if (UBinaryType.BINARY_UNKNOWN == bin.getType ())
		    System.out.println ("  BINARY_UNKNOWN binary");
		else if (UBinaryType.BINARY_IMAGE == bin.getType ())
		{
		    System.out.println ("  BINARY_IMAGE binary");
		    UImage img = bin.getUImage ();
		    System.out.println ("  - width: " + img.getWidth ());
		    System.out.println ("  - heiht: " + img.getHeight ());
		    System.out.println ("  - size: " + img.getSize ());

		    byte[] data = img.getData ();

		    if (UImageFormat.IMAGE_RGB == img.getImageFormat ())
			System.out.println ("  - IMAGE_RGB image");
		    else if (UImageFormat.IMAGE_YCbCr == img.getImageFormat ())
			System.out.println ("  - IMAGE_YCbCr image");
		    else if (UImageFormat.IMAGE_JPEG == img.getImageFormat ())
		    {
			System.out.println ("  - IMAGE_JPEG image");
			try
			{
			    ByteArrayInputStream bais = new ByteArrayInputStream(data);
			    com.sun.image.codec.jpeg.JPEGImageDecoder decoder = com.sun.image.codec.jpeg.JPEGCodec.createJPEGDecoder(bais);
			    BufferedImage buffer = decoder.decodeAsBufferedImage();
			    imageSampler.addImage(buffer, (int) img.getWidth(), (int) img.getHeight());
			    buffer.flush();
			    buffer = null;

			} catch (Exception e)
			{
			}
		    }
		    else if (UImageFormat.IMAGE_PPM == img.getImageFormat ())
			System.out.println ("  - IMAGE_PPM image");
		    else if (UImageFormat.IMAGE_UNKNOWN == img.getImageFormat ())
			System.out.println ("  - IMAGE_UNKNOWN image");
		    else
			System.out.println ("Error: not a valid image type");

		}
		else if (UBinaryType.BINARY_SOUND == bin.getType ())
		    System.out.println ("  BINARY_SOUND binary");
		else
		    System.out.println ("Error: not a valid binary type");
	    }
	    else if (UDataType.DATA_LIST == value.getType ())
	    {
		System.out.println (" DATA_LIST value: ");
		UList l = value.getUList ();
		int i;
		for (i = 0; i < l.size (); ++i)
		{
		    System.out.println (l.getArray ().get (i));
		}
	    }
	    else if (UDataType.DATA_OBJECT == value.getType ())
		System.out.println (" DATA_OBJECT value");
	    else if (UDataType.DATA_VOID == value.getType ())
		System.out.println (" DATA_VOID value");
	    else
		System.out.println ("Error: not a valid value type");
	}
	else
	    System.out.println ("Error: not a message value type");

	return UCallbackAction.URBI_CONTINUE;
    }
}

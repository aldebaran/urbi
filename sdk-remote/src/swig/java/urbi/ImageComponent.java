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

import java.awt.*;
import java.awt.event.*;
import java.awt.image.BufferedImage;
import java.awt.geom.Line2D;

import javax.swing.*;

import urbi.UClient;

/**
 * The ImageComponent class is used to display an image. It subclasses
 * JPanel to take advantage of Swing's double buffering.
 * <p>
 * <p>
 * @author Bastien Saltel
 */

public class	ImageComponent extends JPanel
{
    /** The image to display. */
	private	BufferedImage	image = null;

    /** The width of the image. */
	private	int width = 0;

    /** The height of the image. */
	private int height = 0;

	private double		x = 0;

	private double		y = 0;

	/**
     * Constructor for ImageComponent. Calls the init method to set
	 * the background in white color.
     * <p>
	 */
	public ImageComponent()
	{
		init();
	}

	/**
	 * Sets the width of the image.
     * <p>
	 */
	public void		setWidth(int i)
	{
		width = i;
	}

	/**
	 * Sets the height of the image.
     * <p>
	 */
	public void		setHeight(int i)
	{
		height = i;
	}

	/**
	 * Returns the width of the image.
     * <p>
	 */
	public int		getWidth()
	{
		return width;
	}

	/**
	 * Returns the height of the image.
     * <p>
	 */
	public int		getHeight()
	{
		return height;
	}

	public void		setImage(byte[] buffer)
	{
		ImageUtilities.setWidth(width);
		ImageUtilities.setHeight(height);
		Image im = ImageUtilities.blockingLoad(buffer);
		if (im == null)
			image = null;
		else
			{
				image = ImageUtilities.makeBufferedImage(im);
				if (getGraphics() != null)
					repaint();
				im = null;
			}
	}

	/**
	 * Sets the image by calling the blockingLoad and makeBufferedImage
	 * methods of the ImageUtilities class and displays it.
     * <p>
	 * @param buffer The buffer containing the binary data of the image.
	 * @see ImageUtilities
	 */
	public void		setImage(byte[] buffer, double x, double y)
	{
		ImageUtilities.setWidth(width);
		ImageUtilities.setHeight(height);
		Image im = ImageUtilities.blockingLoad(buffer);
		this.x = x;
		this.y = y;
		if (im == null)
			image = null;
		else
			{
				image = ImageUtilities.makeBufferedImage(im);
				if (getGraphics() != null)
					repaint();
				im = null;
			}
	}

	/**
	 * Sets the image and displays it.
     * <p>
	 * @param im The buffered image.
	 * @see ImageUtilities
	 */
	public void		setImage(BufferedImage im)
	{
		image = im;
		if (getGraphics() != null)
			repaint();
	}

	public void		setImage(BufferedImage im, double x, double y)
	{
		image = im;
		this.x = x;
		this.y = y;
		if (getGraphics() != null)
			repaint();
	}

	/**
	 * Sets the background in white color.
     * <p>
	 */
	private void	init()
	{
		setBackground(Color.white);
	}

	/**
	 * Returns the displayed image.
     * <p>
	 */
	public BufferedImage	getImage()
	{
		return image;
	}

	public void		update(Graphics graphics)
	{
		if (graphics != null)
			{
				graphics.setColor(getBackground());
				graphics.fillRect(0, 0, width, height);
			}
	}

	/**
	 * Displays the image.
     * <p>
	 */
	public void		paint(Graphics graphics)
	{
		if (getImage() != null && graphics != null)
			{
				graphics.drawImage(getImage(), 0, 0, null);
				graphics.setColor(Color.red);
				if (y != 0)
					graphics.drawLine(0, (int)y, width, (int)y);
				if (x != 0)
					graphics.drawLine((int)x, 0, (int)x, height);
			}
	}

	/**
	 * Returns the largest image size.
     * <p>
	 * @return  The largest image size.
	 */
	public Dimension		getPreferredSize()
	{
		return new Dimension(width, height);
	}

}

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

/**
 * The ImageUtilities class is full of handy static methods.
 * <p>
 * <p>
 * @author Bastien Saltel
 */

public class	ImageUtilities
{
	/** The component. */
	private static final	Component component = new Component() {};

   /** The mediatracker.associated with the component */
	private static final	MediaTracker mediatracker = new MediaTracker(component);

    /** The id of the image. */
	private static int id = 0;

    /** The width of the image. */
	private	static int width = 0;

    /** The height of the image. */
	private static int height = 0;

	/**
	 * Sets the width of the image.
     * <p>
	 */
	public static void		setWidth(int i)
	{
		width = i;
	}

	/**
	 * Sets the height of the image.
     * <p>
	 */
	public static void		setHeight(int i)
	{
		height = i;
	}

	/**
	 * Returns the width of the image.
     * <p>
	 */
	public static int		getWidth()
	{
		return width;
	}

	/**
	 * Returns the height of the image.
     * <p>
	 */
	public static int		getHeight()
	{
		return height;
	}

	/**
	 * Waits for the given image to load fully. Returns true if
	 * everything goes well or false if there is an error while loading
	 * the image.
     * <p>
	 * @return true if everything goes well or false if there is an error
	 * during loading the image.
	 */
	public static boolean	waitForImage(Image image)
	{
		int		i;

		synchronized (component)
			{
				i = id++;
			}
		mediatracker.addImage(image, i, width, height);
		try
			{
				mediatracker.waitForID(i);
			}
		catch (InterruptedException ie)
			{
				return false;
			}
		if (mediatracker.isErrorID(i))
			return false;
		return true;
	}

	/**
	 * Loads an image from the given binary buffer and will not return until
	 * the image is fully loaded.
	 * <p>
	 * @param buffer The buffer containing the binary data of the image.
	 * @return  The fully loaded image if everything goes well or null if there
	 * is an error while loading.
	 */
	public static Image		blockingLoad(byte[] buffer)
	{
		Image image = Toolkit.getDefaultToolkit().createImage(buffer);
		if (waitForImage(image) == false)
			return null;
		return image;
	}

	/**
	 * Creates a buffered image from the supplied image.
	 * <p>
	 * @param image  The supplied image.
	 * @return  The buffered image.
	 */
	public static BufferedImage		makeBufferedImage(Image image)
	{
		if (image == null)
			System.out.println("ImageUtilities l.67 Image null");
		return makeBufferedImage(image, BufferedImage.TYPE_INT_RGB);
	}

	/**
	 * Creates a buffered image from the supplied image and its type.
	 * <p>
	 * @param image  The supplied image.
	 * @param imageType The type of the image.
	 * @return  The buffered image.
	 */
	public static BufferedImage		makeBufferedImage(Image image, int imageType)
	{
		if (waitForImage(image) == false)
			return null;
		if (image == null)
			System.out.println("ImageUtilities l.77 Image null");
		BufferedImage	bufferedImage = new BufferedImage(width, height, imageType);

		Graphics2D g = bufferedImage.createGraphics();
		if (image == null)
			System.out.println("ImageUtilities l.80 Image null");
		g.drawImage(image, null, null);
		return bufferedImage;
	}

	/**
	 * Returns a frame subclass whose the update mathod does not clear the frame's
	 * drawing surface.
	 * <p>
	 * @param name  The frame's title.
	 * @param c  The component whose is used to fit the frame's size.
	 * @return	The generated frame.
	 */
	public static Frame		getNonClearingFrame(String name, Component c)
	{
		final Frame f = new Frame(name)
			{
				public void		update(Graphics g)
				{
					paint(g);
				}
			};
		sizeContainerToComponent(f, c);
		centerFrame(f);
		f.setLayout(new BorderLayout());
		f.add(c, BorderLayout.CENTER);
		f.addWindowListener(new WindowAdapter()
			{
				public void windowClosing(WindowEvent e)
				{
					f.dispose();
				}
			});
		return f;
	}

	/**
	 * Resizes the given container to enclose the preferred size of the given component.
	 * <p>
	 * @param container The container to resize.
	 * @param component The component.
	 */
	public static void		sizeContainerToComponent(Container container, Component component)
	{
		if (container.isDisplayable() == false)
			container.addNotify();
		Insets insets = container.getInsets();
		Dimension size = component.getPreferredSize();
		int		w = insets.left + insets.right + size.width;
		int		h = insets.top + insets.bottom + size.height;
		container.setSize(w, h);
	}

	/**
	 * Places the given frame in the center of the screen.
	 */
	public static void		centerFrame(Frame f)
	{
		Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
		Dimension d = f.getSize();
		int		x = (screen.width - d.width) / 2;
		int		y = (screen.height - d.height) / 2;
		f.setLocation(x, y);
	}
}

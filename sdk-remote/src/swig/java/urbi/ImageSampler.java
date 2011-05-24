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
import java.awt.color.*;
import java.awt.event.*;
import java.awt.geom.AffineTransform;
import java.awt.image.*;
import java.util.*;

/**
 * The ImageSampler class displays a list of image processing operations.
 * A first frame contains the source image and a second frame contains
 * the user controls. If we want to process operations on the source image,
 * a third frame is created contained the last processed image.
 * <p>
 * <p>
 * @author Bastien Saltel
 */

public class	ImageSampler extends Frame
{
    /** The frame containing the source image. */
	private	final Frame	imageFrame;

	/** The frame containing the processed image. */
	private	Frame	imageFrame2 = null;

    /** The image component associated with the source image. */
	private final ImageComponent		imageComponent;

   /** The image component associated with the processed image. */
	private ImageComponent		imageComponent2 = null;

	/** The container of the image processing operators. */
	private Hashtable		ops;

    /** The last used image processing operator. */
	private	BufferedImageOp	op = null;

	/**
     * Constructor for ImageSampler. Displays the frame containing the user controls,
	 * registers all the image processing operators and allocates the image component
	 * associated with the source image.
     * <p>
	 */
	public ImageSampler()
	{
		super("Image Sampler");
		imageFrame = new Frame("Image");
		imageFrame.setLayout(new BorderLayout());
		ImageUtilities.centerFrame(imageFrame);
		createOps();
		createUI();
		setVisible(true);
		imageFrame.setVisible(true);
		imageComponent = new ImageComponent();
	}

	/**
	 * Creates a table of image processing operators, calls createConvolutions,
	 * createTransformations, createLookups and createRescales methods to create
	 * a cornucopia of image operators.
	 * <p>
	 */
	private void	createOps()
	{
		ops = new Hashtable();
		createConvolutions();
		createTransformations();
		createLookups();
		createRescales();
		createColorOps();
	}

	/**
	 * Creates convolutions operators.
	 * <p>
	 */
	private void	createConvolutions()
	{
		float	ninth = 1.0f / 9.0f;
		float[]	blurKernel =
			{
				ninth, ninth, ninth,
				ninth, ninth, ninth,
				ninth, ninth, ninth
			};
		ops.put("Blur", new ConvolveOp(new Kernel(3, 3, blurKernel),
									   ConvolveOp.EDGE_NO_OP, null));
		float[]	sharp =
			{
				0f, -1f, 0f,
				-1f, 5f, -1f,
				0f, -1f, 0f
			};
		ops.put("Sharpen", new ConvolveOp(new Kernel(3, 3, sharp)));
	}

	/**
	 * Creates rotating and scaling operators.
	 * <p>
	 */
	private void	createTransformations()
	{
		AffineTransform	at;
		at = AffineTransform.getRotateInstance(Math.PI / 6, 0, 285);
		ops.put("Rotate nearest neighbor", new AffineTransformOp(at, null));

		RenderingHints	rh = new RenderingHints(RenderingHints.KEY_INTERPOLATION,
												RenderingHints.VALUE_INTERPOLATION_BILINEAR);
		ops.put("Rotate bilinear", new AffineTransformOp(at, null));

		at = AffineTransform.getScaleInstance(.5, .5);
		ops.put("Scale .5, .5", new AffineTransformOp(at, null));

		at = AffineTransform.getRotateInstance(Math.PI / 6);
		ops.put("Rotate bilinear (origin)", new AffineTransformOp(at, rh));
	}

	/**
	 * Creates brightening, posterizing, color inverting and removing operators.
	 * <p>
	 */
	private void	createLookups()
	{
		short[]	brighten = new short[256];
		short[]	betterBrighten = new short[256];
		short[]	posterize = new short[256];
		short[]	invert = new short[256];
		short[]	straight = new short[256];
		short[]	zero = new short[256];

		for (int i = 0; i < 256; i++)
			{
				brighten[i] = (short)(128 + i / 2);
				betterBrighten[i] = (short)(Math.sqrt((double)i / 255.0) * 255.0);
				posterize[i] = (short)(i - (i % 32));
				invert[i] = (short)(255 - i);
				straight[i] = (short)i;
				zero[i] = (short)0;
			}
		ops.put("Brighten", new LookupOp(new ShortLookupTable(0, brighten), null));
		ops.put("Better Brighten", new LookupOp(new ShortLookupTable(0, betterBrighten), null));
		ops.put("Posterize", new LookupOp(new ShortLookupTable(0, posterize), null));
		ops.put("Invert", new LookupOp(new ShortLookupTable(0, invert), null));

		short[][]		redOnly = {invert, straight, straight};
		short[][]		greenOnly = {straight, invert, straight};
		short[][]		blueOnly = {straight, straight, invert};

		ops.put("Red invert", new LookupOp(new ShortLookupTable(0, redOnly), null));
		ops.put("Green invert", new LookupOp(new ShortLookupTable(0, greenOnly), null));
		ops.put("Blue invert", new LookupOp(new ShortLookupTable(0, blueOnly), null));

		short[][]		redRemove = {zero, straight, straight};
		short[][]		greenRemove = {straight, zero, straight};
		short[][]		blueRemove = {straight, straight, zero};

		ops.put("Red remove", new LookupOp(new ShortLookupTable(0, redRemove), null));
		ops.put("Green remove", new LookupOp(new ShortLookupTable(0, greenRemove), null));
		ops.put("Blue remove", new LookupOp(new ShortLookupTable(0, blueRemove), null));
	}

	/**
	 * Creates rescaling operators.
	 * <p>
	 */
	private void	createRescales()
	{
		ops.put("Rescale .5, 0", new RescaleOp(.5f, 0, null));
		ops.put("Rescale .5, 64", new RescaleOp(.5f, 64, null));
		ops.put("Rescale 1.2, 0", new RescaleOp(1.2f, 0, null));
		ops.put("Rescale 1.5, 0", new RescaleOp(1.5f, 0, null));
	}

	/**
	 * Creates color scaling operators.
	 * <p>
	 */
	private void	createColorOps()
	{
		ops.put("Grayscale", new ColorConvertOp(ColorSpace.getInstance(ColorSpace.CS_GRAY), null));
	}

	/**
	 * Adds a new image to the frame.
	 * <p>
	 * @param buffer The buffer containing the binary data of the image.
	 * @param width  The width of the image.
	 * @param height The height of the image.
	 */
	public synchronized void	addImage(byte[] buffer, int width, int height)
	{
		imageComponent.setWidth(width);
		imageComponent.setHeight(height);
		imageComponent.setImage(buffer);
		imageFrame.add(imageComponent, BorderLayout.CENTER);
		ImageUtilities.sizeContainerToComponent(imageFrame, imageComponent);
		if (imageFrame2 != null && imageComponent2 != null && imageComponent.getImage() != null)
			{
				imageComponent2.setWidth(width);
				imageComponent2.setHeight(height);
				imageComponent2.setImage(op.filter(imageComponent.getImage(), null));
				imageFrame2.add(imageComponent2, BorderLayout.CENTER);
				ImageUtilities.sizeContainerToComponent(imageFrame2, imageComponent2);
			}
		imageFrame.setVisible(true);
	}

	public synchronized void	addImage(byte[] buffer, int width, int height, double x, double y)
	{
		imageComponent.setWidth(width);
		imageComponent.setHeight(height);
		imageComponent.setImage(buffer, x, y);
		imageFrame.add(imageComponent, BorderLayout.CENTER);
		ImageUtilities.sizeContainerToComponent(imageFrame, imageComponent);
		if (imageFrame2 != null && imageComponent2 != null && imageComponent.getImage() != null)
			{
				imageComponent2.setWidth(width);
				imageComponent2.setHeight(height);
				imageComponent2.setImage(op.filter(imageComponent.getImage(), null));
				imageFrame2.add(imageComponent2, BorderLayout.CENTER);
				ImageUtilities.sizeContainerToComponent(imageFrame2, imageComponent2);
			}
		imageFrame.setVisible(true);
	}

	public synchronized void	addImage(BufferedImage buffer, int width, int height)
	{
		imageComponent.setWidth(width);
		imageComponent.setHeight(height);
		imageComponent.setImage(buffer);
		imageFrame.add(imageComponent, BorderLayout.CENTER);
		ImageUtilities.sizeContainerToComponent(imageFrame, imageComponent);
		if (imageFrame2 != null && imageComponent2 != null && imageComponent.getImage() != null)
			{
				imageComponent2.setWidth(width);
				imageComponent2.setHeight(height);
				imageComponent2.setImage(op.filter(imageComponent.getImage(), null));
				imageFrame2.add(imageComponent2, BorderLayout.CENTER);
				ImageUtilities.sizeContainerToComponent(imageFrame2, imageComponent2);
			}
		imageFrame.setVisible(true);
	}

	public synchronized void	addImage(BufferedImage buffer, int width, int height, double x, double y)
	{
		imageComponent.setWidth(width);
		imageComponent.setHeight(height);
		imageComponent.setImage(buffer, x, y);
		imageFrame.add(imageComponent, BorderLayout.CENTER);
		ImageUtilities.sizeContainerToComponent(imageFrame, imageComponent);
		if (imageFrame2 != null && imageComponent2 != null && imageComponent.getImage() != null)
			{
				imageComponent2.setWidth(width);
				imageComponent2.setHeight(height);
				imageComponent2.setImage(op.filter(imageComponent.getImage(), null));
				imageFrame2.add(imageComponent2, BorderLayout.CENTER);
				ImageUtilities.sizeContainerToComponent(imageFrame2, imageComponent2);
			}
		imageFrame.setVisible(true);
	}

	/**
	 * Creates the user interface to process the image operations.
	 * <p>
	 */
	private void	createUI()
	{
		setFont(new Font("Serif", Font.PLAIN, 12));
		setLayout(new BorderLayout());

		// Set our location to the left of the image frame.
		setSize(200, 350);
		Point	pt = imageFrame.getLocation();
		setLocation(pt.x - getSize().width, pt.y);

		final Label		statusLabel = new Label("");

		// Make a sorted list of the operators.
		Enumeration e = ops.keys();
		Vector names = new Vector();
		while (e.hasMoreElements())
			names.addElement(e.nextElement());
		Collections.sort(names);

		final java.awt.List		list = new java.awt.List();
		for (int i = 0; i < names.size(); i++)
			list.add((String)names.elementAt(i));
		add(list, BorderLayout.CENTER);

		// When an item is selected, apply the corresponding transformation
		list.addItemListener(new ItemListener()
			{
				public void		itemStateChanged(ItemEvent ie)
				{
					if (imageComponent == null)
						return ;
					if (ie.getStateChange() != ItemEvent.SELECTED)
						return ;
					String	key = list.getSelectedItem();

					op = (BufferedImageOp)ops.get(key);

					if (imageFrame2 == null)
						{
							imageFrame2 = new Frame("Image Transformed");
							imageFrame2.setLayout(new BorderLayout());
							ImageUtilities.sizeContainerToComponent(imageFrame2, imageComponent);
							Point	pt = imageFrame.getLocation();
							imageFrame2.setLocation(pt.x, pt.y + imageFrame2.getSize().height);
							imageFrame2.setVisible(true);
							imageComponent2 = new ImageComponent();
						}

					list.setEnabled(false);
					if (imageComponent.getImage() != null)
						{
							imageComponent2.setWidth(imageComponent.getWidth());
							imageComponent2.setHeight(imageComponent.getHeight());
							imageComponent2.setImage(op.filter(imageComponent.getImage(), null));
						}
					imageComponent2.setSize(imageComponent2.getPreferredSize());
					imageFrame2.setSize(imageFrame2.getPreferredSize());
					list.setEnabled(true);
					statusLabel.setText("Performing " + key + "...done.");
				}
			});

		Panel bottom = new Panel(new GridLayout(2, 1));
		bottom.add(statusLabel);
		add(bottom, BorderLayout.SOUTH);

		addWindowListener(new WindowAdapter()
			{
				public void windowClosing(WindowEvent e)
				{
					imageFrame.dispose();
					if (imageFrame2 != null)
						imageFrame2.dispose();
					dispose();
					System.exit(0);
				}
			});
	}
}


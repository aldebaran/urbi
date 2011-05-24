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

/**
 * The ImageFilter class is a useful class to obtain the RGB and YCrCb values of
 * a given color.
 * <p>
 * <p>
 * @author Bastien Saltel
 */

public class	ImageFilter
{
	/**
	 * Returns the Red value of the color.
     * <p>
	 * @param color The given color.
	 * @return  The Red value
	 */
	public static double	getRed(int color)
	{
		return (color >> 16) & 0xFF;
	}

	/**
	 * Returns the Green value of the color.
     * <p>
	 * @param color The given color.
	 * @return  The Green value
	 */
	public static double	getGreen(int color)
	{
		return (color >> 8) & 0xFF;
	}

	/**
	 * Returns the Blue value of the color.
     * <p>
	 * @param color The given color.
	 * @return  The Blue value
	 */
	public static double	getBlue(int color)
	{
		return color & 0xFF;
	}

	/**
	 * Returns the Y value of the color.
     * <p>
	 * @param color The given color.
	 * @return  The Y value
	 */
	public static double	getY(int color)
	{
		return (0.299 * getRed(color) + 0.587 * getGreen(color) + 0.114 * getBlue(color));
	}

	/**
	 * Returns the Cr value of the color.
     * <p>
	 * @param color The given color.
	 * @return  The Cr value
	 */
	public static double	getCr(int color)
	{
		return (0.500 * getRed(color) - 0.419 * getGreen(color) - 0.081 * getBlue(color));
	}

	/**
	 * Returns the Cb value of the color.
     * <p>
	 * @param color The given color.
	 * @return  The Cb value
	 */
	public static double	getCb(int color)
	{
		return (-0.169 * getRed(color) - 0.331 * getGreen(color) + 0.500 * getBlue(color));
	}
}

/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

package examples.uobjectjava;

import urbi.*;
import urbi.*;
import java.lang.Math;


/// The Colormap UObject already exist in C++ under the name "colormap"
/// This UObject is here only to illustrate the creation of UObjects in
/// Java.
/// If you really need the feature colormap, use the C++ uobject that is
/// already available in urbi server as a plugged UObject (much faster)
/// NB: to create a C++ colormap UObject do
/// c = new colormap (....  /// note the 'c' minuscule of 'c'olormap
///
/// Colormap is used to do ball detection.

public class Colormap extends UObject
{
    /// Register your UObject (so that urbi knows about it)
    static { UStart(Colormap.class); };


    /// x position of the color blob in the image
    private UVar x = new UVar ();
    // y position of the color blob in the image
    private UVar y = new UVar ();
    /// is a blob of color visible in the image
    private UVar visible = new UVar ();
    /// ratio: (size blob) / (size image)
    private UVar ratio = new UVar ();
    /// threshold used to determine if the blob is visible or not
    private UVar threshold = new UVar ();

    /// Color of the blob
    private UVar ymin = new UVar ();
    private UVar ymax = new UVar ();
    private UVar cbmin = new UVar ();
    private UVar cbmax = new UVar ();
    private UVar crmin = new UVar ();
    private UVar crmax = new UVar ();


    /// Java constructor
    public Colormap (String str) {
	super (str);
	UBindFunction (this, "init");
    }

    /// Urbi constructor
    public int init(String source, /// We expect the name of a camera variable or any UImage variable
		    double _Ymin,
		    double _Ymax,
		    double _Cbmin,
		    double _Cbmax,
		    double _Crmin,
		    double _Crmax,
		    double _threshold)
    {
	UBindVar(x, "x");
	UBindVar(y, "y");
	UBindVar(visible, "visible");
	UBindVar(ratio, "ratio");
	UBindVar(threshold, "threshold");
	UBindVar(ymin, "ymin");
	UBindVar(ymax, "ymax");
	UBindVar(cbmin, "cbmin");
	UBindVar(cbmax, "cbmax");
	UBindVar(crmin, "crmin");
	UBindVar(crmax, "crmax");

	UNotifyChange(source, "newImage");

	// initialization
	ymin.setValue (_Ymin);
	ymax.setValue (_Ymax);
	cbmin.setValue (_Cbmin);
	cbmax.setValue (_Cbmax);
	crmin.setValue (_Crmin);
	crmax.setValue (_Crmax);
	threshold.setValue (_threshold);
	x.setValue (-1);
	y.setValue (-1);
	visible.setValue (0);
	ratio.setValue (0);
	return 0;
    }


    public int newImage(UVar img)
    {
	if (getLoad ().doubleValue () < 0.5)
	    return 1;

	UImage img1 = img.uimageValue ();  //ptr copy
	long w = img1.getWidth ();
	long h = img1.getHeight ();

	//lets cache things
	int ymax = this.ymax.intValue ();
	int ymin = this.ymin.intValue ();
	int crmin = this.crmin.intValue ();
	int crmax = this.crmax.intValue ();
	int cbmin = this.cbmin.intValue ();
	int cbmax = this.cbmax.intValue ();

	long x=0,y=0,xx=0,yy=0,xy=0;
	int size = 0;

	byte[] data = img1.getData ();
	for (int i = 0; i < w; i++)
	    for (int j = 0; j < h; j++) {

		int lum = data[(int) (i+j*w)*3] & 0xff;
		int cb  = data[(int) (i+j*w)*3+1] & 0xff;
		int cr  = data[(int) (i+j*w)*3+2] & 0xff;

		if ( (lum  >= ymin) &&
		     (lum  <= ymax) &&
		     (cb >= cbmin) &&
		     (cb <= cbmax) &&
		     (cr >= crmin) &&
		     (cr <= crmax) ) {
		    size++;
		    x += i;
		    y += j;
		    xx += i*i;
		    yy += j*j;
		    xy += i*j;
		}
	    }

	this.ratio.setValue ((double)size / (double)(w*h));
	if (size > (int)(threshold.doubleValue () * (double)(w*h)))
	{
	    this.visible.setValue (1);
	    this.x.setValue (0.5 - ((double)x / ((double)size * (double)w)));
	    this.y.setValue (0.5 - ((double)y / ((double)size * (double)h)));
	}
	else {
	    this.x.setValue (-1);
	    this.y.setValue (-1);
	    this.visible.setValue (0);
	}

	return 1;
    }
}

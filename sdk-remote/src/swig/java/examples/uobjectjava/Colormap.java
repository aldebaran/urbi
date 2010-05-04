package examples.uobjectjava;

import liburbi.main.*;
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
/// In Webots, with a Nao, you can try:
///
/// ball = new Colormap("camera.ycbcr",0, 255, 50, 90, 160,210, 0.0015);
/// var ball.a = 50;

/// balltracking : whenever (ball.visible) {
///     HeadYaw.val  = HeadYaw.val  + ball.a * camera.xfov * ball.x &
///     HeadPitch.val = HeadPitch.val - ball.a * camera.yfov * ball.y &
/// }
///
/// Then move the orange ball with the mouse in front of the Nao camera,
/// and the Nao head will follow it
///
public class Colormap extends UObjectJava
{
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

	try {
	    UBindFunction (this, "init");
	}
	catch (Exception e) {
	    System.out.println (e);
	}
    }

    /// Urbi constructor
    public int init(UValue source, /// We expect the name of a camera variable or any UImage variable
		    UValue _Ymin, /// double
		    UValue _Ymax, /// double
		    UValue _Cbmin, /// double
		    UValue _Cbmax, /// double
		    UValue _Crmin, /// double
		    UValue _Crmax, /// double
		    UValue _threshold) /// double
    {
	try {

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

	    UNotifyChange(source.getString (), "newImage");

	    // initialization
	    ymin.set (_Ymin);
	    ymax.set (_Ymax);
	    cbmin.set (_Cbmin);
	    cbmax.set (_Cbmax);
	    crmin.set (_Crmin);
	    crmax.set (_Crmax);
	    threshold.set (_threshold);
	    x.set (-1);
	    y.set (-1);
	    visible.set (0);
	    ratio.set (0);
	}
	catch (Exception e) {
	    System.out.println (e);
	    return 1;
	}
	return 0;
    }


    public int newImage(UVar img)
    {
	if (getLoad ().getDouble () < 0.5)
	    return 1;

	UImage img1 = img.getUImage ();  //ptr copy
	long w = img1.getWidth ();
	long h = img1.getHeight ();

	//lets cache things
	int ymax = this.ymax.getInt ();
	int ymin = this.ymin.getInt ();
	int crmin = this.crmin.getInt ();
	int crmax = this.crmax.getInt ();
	int cbmin = this.cbmin.getInt ();
	int cbmax = this.cbmax.getInt ();

	long x=0,y=0,xx=0,yy=0,xy=0;
	int size = 0;

	byte[] data = img1.getDataAsByte ();
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

	this.ratio.set ((double)size / (double)(w*h));
	if (size > (int)(threshold.getDouble () * (double)(w*h)))
	{
	    this.visible.set (1);
	    this.x.set (0.5 - ((double)x / ((double)size * (double)w)));
	    this.y.set (0.5 - ((double)y / ((double)size * (double)h)));
	}
	else {
	    this.x.set (-1);
	    this.y.set (-1);
	    this.visible.set (0);
	}

	return 1;
    }
}

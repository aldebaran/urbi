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

import urbi.UObject;
import urbi.UVar;
import urbi.UValue;

public class Adder extends UObject // must extends UObject
{
    /// Register your UObject (so that urbi knows about it)
    static { UStart(Adder.class); };

    /// Declare a variable v that will be accessible in Urbi
    private UVar v = new UVar ();

    /// Constructor
    public Adder (String s) {
    	super (s);
	UBindFunction (this, "init");
    }

    /// The init function is the constructor in Urbi. Here it takes
    /// one argument that we use to initialise the 'v' variable.
    /// The init function must return an int of value 0
    /// if all went OK.
    public int init (double v_init) {
	/// Bind the variable v to Urbi
	UBindVar (v, "v");
	/// Initialise our UVar v to the value given in the
	/// constructor
	v.setValue(v_init);
	/// Bind the function add to Urbi
	UBindFunction (this, "add");
	return 0;
    }

    public double add (double rhs) {
    	/// Return the value of our UVar v (converted to double)
    	/// plus the value of the argument of the function.
    	return v.doubleValue () + rhs;
    }
}


/*

/// No Urbi Constructor version:
/// ----------------------------

public class Adder extends UObject // must extends UObject
{
    /// Register your UObject (so that urbi knows about it)
    static { UStart(Adder.class); };

    /// Declare a variable v that will be accessible in Urbi
    private UVar v = new UVar ();

    /// Constructor
    public Adder (String s) {
    	super (s);
	/// Bind the variable v to Urbi
	UBindVar (v, "v");
	/// Initialise our UVar v to some value
	/// (we choose 42 :)
	v.setValue(42);
	/// Bind the function add to Urbi
	UBindFunction (this, "add");
    }

    /// The binded function can take no other type than UValue as
    /// parameters.
    public double add (double rhs) {
    	/// Return the value of our UVar v (converted to double)
    	/// plus the value of the argument of the function.
    	return v.doubleValue () + rhs;
    }
}


*/

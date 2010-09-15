package examples.uobjectjava;

import urbi.UObjectJava;
import urbi.generated.UVar;
import urbi.generated.UValue;

public class Adder extends UObjectJava // must extends UObjectJava
{
    /// Declare a variable v that will be accessible in Urbi
    private UVar v = new UVar ();

    /// Constructor
    public Adder (String s) {
    	super (s);

    	try
    	{
	    UBindFunction (this, "init");
    	}
    	catch (Exception e)
    	{
	    System.out.println (e);
    	}
    }

    /// The init function is the constructor in Urbi. Here it takes
    /// one argument that we use to initialise the 'v' variable.
    /// The init function must return an int of value 0
    /// if all went OK.
    public int init (UValue v_init) {

    	try
    	{
	    /// Bind the variable v to Urbi
	    UBindVar (v, "v");

	    /// Initialise our UVar v to the value given in the
	    /// constructor
	    v.set(v_init.getDouble ());

	    /// Bind the function add to Urbi
	    UBindFunction (this, "add");

    	}
    	catch (Exception e)
    	{
	    System.out.println (e);
    	}

	return 0;
    }

    /// The binded function can take no other type than UValue as
    /// parameters.
    public double add (UValue rhs) {
    	/// Convert the UValue to a double
    	double drhs = rhs.getDouble ();
    	/// Return the value of our UVar v (converted to double)
    	/// plus the value of the argument of the function.
    	return v.getDouble () + drhs;
    }
}


/*

/// No Urbi Constructor version:
/// ----------------------------

public class Adder extends UObjectJava // must extends UObjectJava
{
    /// Declare a variable v that will be accessible in Urbi
    private UVar v = new UVar ();

    /// Constructor
    public Adder (String s) {
    	super (s);

    	try
    	{
    		/// Bind the variable v to Urbi
    		UBindVar (v, "v");

    		/// Initialise our UVar v to some value
    		/// (we choose 42 :)
    	 	v.set(42);

    		/// Bind the function add to Urbi
    		UBindFunction (this, "add");
    	}
    	catch (Exception e)
    	{
    		System.out.println (e);
    	}
    }

    /// The binded function can take no other type than UValue as
    /// parameters.
    public double add (UValue rhs) {
    	/// Convert the UValue to a double
    	double drhs = rhs.getDouble ();
    	/// Return the value of our UVar v (converted to double)
    	/// plus the value of the argument of the function.
    	return v.getDouble () + drhs;
    }
}


*/

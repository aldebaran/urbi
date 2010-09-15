
package examples.uobjectjava;

/// Import the liburbi classes in liburbi.main (located in in liburbi.jar).
import urbi.*;
import java.util.Map;

/// Main class.
///  - Load the 'urbijava' library
///  - Starts all the UObjects
///  - Call UObject.main
///
/// Please reuse this class in your Java UObjects.
/// you only need to replace the UObjects names in
///
///  'UObject.UStart'
///
/// by the names of your own UObjects.
///
public class Main {

    static {
	// DEBUG INFORMATIONS
        // get the system environment variables
        System.out.println("Environment Variables");
        System.out.println("*********************");
        Map<String, String> envMap = System.getenv();

        // print the system environment variables
        for (String key : envMap.keySet())
            System.out.println(key + " = " +  envMap.get(key));
        System.out.println("Java library path");
        System.out.println("*****************");
        String lib_path = System.getProperty("java.library.path");
        System.out.println(lib_path);

	/// Load the c++ native library.
	System.loadLibrary("urbijava");
    }

    public static void main(String argv[]) {
	try
	{
	    /// Start the 'Feature1' UObject
	    UObject.UStart (Feature1.class);

	    /// Start the 'Feature2' UObject
	    UObject.UStart (Feature2.class);

	    /// Start the 'Feature3' UObject
	    UObject.UStart (Feature3.class);

	    /// Start the 'Feature4' UObject
	    UObject.UStart (Feature4.class);

	    /// Start the 'Feature6' UObject
	    UObject.UStart (Feature5.class);

	    /// Start the 'Colormap' UObject
	    UObject.UStart (Colormap.class);

	    /// Start the 'Adder' UObject
	    UObject.UStart (Adder.class);

	    /// Start the 'SimpleUObject' UObject
	    UObject.UStart (SimpleUObject.class);

	    /// Call the main in UObject. This main
	    /// will never return.
	    UObject.main (argv);
	}
	catch (Exception e)
	{
	    System.out.println (e);
	}
    }
}

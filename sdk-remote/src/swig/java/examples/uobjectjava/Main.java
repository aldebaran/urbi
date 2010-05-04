
package examples.uobjectjava;

/// Import the liburbi classes in liburbi.main (located in in liburbi.jar).
import liburbi.main.*;
import java.util.Map;

/// Main class.
///  - Load the 'urbijava' library
///  - Starts all the UObjects
///  - Call UObjectJava.main
///
/// Please reuse this class in your Java UObjects.
/// you only need to replace the UObjects names in
///
///  'UObjectJava.UStart'
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
	    /// Start the 'Feature' UObject
	    UObjectJava.UStart (Feature.class);

	    /// Start the 'Colormap' UObject
	    UObjectJava.UStart (Colormap.class);

	    /// Start the 'Adder' UObject
	    UObjectJava.UStart (Adder.class);

	    /// Start the 'SimpleUObject' UObject
	    UObjectJava.UStart (SimpleUObject.class);

	    /// Call the main in UObjectJava. This main
	    /// will never return.
	    UObjectJava.main (argv);
	}
	catch (Exception e)
	{
	    System.out.println (e);
	}
    }
}

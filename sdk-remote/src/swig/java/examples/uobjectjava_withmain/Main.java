/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

package examples.uobjectjava_withmain;

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
	try
	{
	    /// Load the c++ native library.
	    System.loadLibrary("urbijava");
	}
	catch (java.lang.UnsatisfiedLinkError e)
	{
	    System.out.println ("Loading exception: " + e);

	    // DEBUG INFORMATIONS
	    System.out.println();
	    System.out.println("----------- Java.library.path -------------");
	    String lib_path = System.getProperty("java.library.path");
	    System.out.println(lib_path);

	    // get the system environment variables
	    System.out.println();
	    System.out.println("---------- ENVIRONMENT VARIABLES ----------");
	    Map<String, String> envMap = System.getenv();
	    // print the system environment variables
	    for (String key : envMap.keySet())
		System.out.println(key + " = " +  envMap.get(key));
	    System.out.println();

	    System.exit (1);
	}
    }

    public static void main(String argv[]) {
       try
       {
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

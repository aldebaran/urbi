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


/// Import the liburbi classes in liburbi.main (located in in liburbi.jar).
import urbi.UObject;
import java.util.Map;
import java.lang.UnsatisfiedLinkError;
import java.io.File;
import java.io.IOException;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.jar.JarInputStream;
import java.util.jar.JarEntry;

public class UMain
{

    static
    {
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

    // The methods addFile and associated final Class[] parameters
    // were gratefully copied from anthony_miguel @
    // http://forum.java.sun.com/thread.jsp?forum=32&thread=300557&tstart=0&trange=15
    private static final Class[] parameters = new Class[]{URL.class};

    public static void addFile(String s) throws IOException
    {
	addURL(new URL("file", null, s));
    }

    public static void addURL(URL u) throws IOException
    {
	URLClassLoader sysloader = (URLClassLoader)ClassLoader.getSystemClassLoader();
	Class sysclass = URLClassLoader.class;
	try {
	    Method method = sysclass.getDeclaredMethod("addURL",parameters);
	    method.setAccessible(true);
	    method.invoke(sysloader,new Object[]{ u });
	} catch (Throwable t) {
	    t.printStackTrace();
	    throw new IOException("Error, could not add URL to system classloader");
	}//end try catch
    }//end method

    public static void usage()
    {
	System.out.println("usage:");
	System.out.println("<java command line> [OPTIONS] MODULES_NAMES ... [-- [urbi-launch options]]");
	System.out.println();
	System.out.println("Options:");
	System.out.println("  -h, --help  output this message and exit successfully");
	System.out.println();
	System.out.println("MODULE_NAMES is a list of JAR and ZIP archives containings UObjects.");
	System.exit(0);
    }

    public static void main(String argv[])
    {
	boolean done = false;
	int i = 0;
        String uob_path_s = System.getenv("URBI_UOBJECT_PATH");
	// If URBI_UOBJECT_PATH is not defined, look in .
        if (uob_path_s == null)
            uob_path_s = ".";
        String[] uob_path = uob_path_s.split(":");
	for (; i < argv.length && !done; ++i)
	{
	    String s = argv[i];
	    if (s.equals("-h") || s.equals("--help"))
		usage();
	    else if (s.equals("--"))
		done = true;
	    else
	    {
		try {
		    String jarname = argv[i];
		    boolean found = false;
		    // Test if path is absolute
		    if (new File(jarname).getAbsolutePath().equals(jarname)){
			if ((new File(jarname)).exists())
			    found = true;
		    }
		    else
			// Search for uobject in URBI_UOBJECT_PATH
			for(String p: uob_path) {
                            String possible_path = String.format("%s/%s", p, jarname);
			    if ((new File(possible_path)).exists()) {
				jarname = possible_path;
				found = true;
				break;
			    }
			}
		    if (!found) {
			String msg = "JAR archive %s not found (is URBI_UOBJECT_PATH correctly set ?).";
			msg = String.format(msg, jarname);
			throw new FileNotFoundException(msg);
		    }
		    Log.info("Processing " + jarname);
		    addFile(jarname);
		    JarInputStream jis = new JarInputStream(new FileInputStream(jarname));
		    JarEntry entry = jis.getNextJarEntry();
		    while (entry != null) {
			String name = entry.getName();
			if (name.endsWith(".class")) {
			    name = name.substring(0, name.length() - 6);
			    name = name.replace('/', '.');
			    Class.forName(name);
			    Log.info("'" + name + "' loaded");
			}
			entry = jis.getNextJarEntry();
		    }
		} catch (ClassNotFoundException cnfe) {
		    Log.error(cnfe.toString());
                    System.exit(1);
		} catch (Exception e) {
		    Log.error(e.toString());
                    System.exit(1);
		}
	    }
	}
	String[] subargv = null;
	if (i < argv.length)
	{
	    subargv = new String[argv.length - i];
	    System.arraycopy(argv, i, subargv, 0, subargv.length);
	}
	else
	    subargv = new String[0];
	try
	{
	    UObject.main(subargv);
	}
	catch (Exception e)
	{
	    Log.error(e.toString());
            System.exit(1);
	}
    }
}

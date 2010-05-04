package gnu.bytecode;
import java.util.Hashtable;
import java.net.URL;

/** Load classes from a set of byte arrays.
 * @author	Per Bothner
 */

public class ArrayClassLoader extends ClassLoader
{
  Hashtable map = new Hashtable(100);

  /** If non-null, context to use for finding resources. */
  URL context;

  public ArrayClassLoader ()
  {
  }

  public ArrayClassLoader (ClassLoader parent)
  {
    super(parent);
  }

  /** Get base URL to use for finding resources, or null if none is set. */
  public URL getResourceContext () { return context; }

  /** Set base URL to use for finding resources. */
  public void setResourceContext (URL context) { this.context = context; }

  /** Load classes from the given byte arrays.
    By convention, the classes we manage are named "lambda"+<INTEGER>. */
  public ArrayClassLoader (byte[][] classBytes)
  {
    for (int i = classBytes.length;  --i >= 0; )
      addClass("lambda" + i, classBytes[i]);
  }

  public ArrayClassLoader (String[] classNames, byte[][] classBytes)
  {
    for (int i = classBytes.length;  --i >= 0; )
      addClass(classNames[i], classBytes[i]);
  }

  public void addClass(Class clas)
  {
    map.put(clas.getName(), clas);
  }

  public void addClass(String name, byte[] bytes)
  {
    map.put(name, bytes);
  }

  public void addClass (ClassType ctype)
    throws java.io.IOException
  {
    if ((ctype.flags & ClassType.EXISTING_CLASS) != 0)
      addClass(ctype. getReflectClass());
    else
      addClass(ctype.getName(), ctype.writeToArray());
  }

  protected URL findResource(String name)
  {
    if (context != null)
      {
        try
          {
            URL url = new URL(context, name);
            url.openConnection().connect();
            return url;
          }
        catch (Throwable ex)
          {
            // Fall through ...
          }
      }
    return super.findResource(name);
  }

  /* #ifdef JAVA2 */
  public Class findClass (String name)
  /* #else */
  // public Class loadClass (String name, boolean resolve)
  /* #endif */
    throws ClassNotFoundException
  {
    Object r = map.get(name);
    Class clas;
    if (r == null)
      {
        /* #ifdef JAVA2 */
        throw new ClassNotFoundException(name);
        /* #else */
        // clas = Class.forName(name);
        /* #endif */
      }
    else if (r instanceof byte[])
      {
        // double-locking? is this safe?  or needed? FIXME.
	synchronized (this)
	  {
	    r = map.get(name);
	    if (r instanceof byte[])
	      {
		byte[] bytes = (byte[]) r;
		clas = defineClass (name, bytes, 0, bytes.length);
		map.put(name, clas);
	      }
	    else
	      clas = (Class) r;
	  }
      }
    else
      clas = (Class) r;
    /* #ifndef JAVA2 */
    // if (resolve && clas != null)
    //   resolveClass (clas);
    /* #endif */
    return clas;
  }
}

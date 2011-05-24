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

import java.lang.StackTraceElement;
import java.lang.Exception;

public class Log
{
    private static String default_category = "UObjectJava";

    public static native void info(String category,
                                   String msg,
                                   String functionname,
                                   String filename,
                                   int linenumber);

    public static void info(String category, String msg)
    {
        StackTraceElement elt = new Exception().getStackTrace()[0];
        info(category, msg,
             elt.getClassName() + "." + elt.getMethodName(),
             elt.getFileName(),
             elt.getLineNumber());
    }

    public static void info(String msg)
    {
        info(default_category, msg);
    }

    public static native void error(String category,
                                    String msg,
                                    String functionname,
                                    String filename,
                                    int linenumber);

    public static void error(String category, String msg)
    {
        StackTraceElement elt = new Exception().getStackTrace()[0];
        error(category, msg,
              elt.getClassName() + "." + elt.getMethodName(),
              elt.getFileName(),
              elt.getLineNumber());
    }

    public static void error(String msg)
    {
        error(default_category, msg);
    }
}

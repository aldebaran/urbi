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

public class SimpleUObject extends UObject {

    /// Register your UObject (so that urbi knows about it)
    static { UStart(SimpleUObject.class); };


   /// The constructor of your UObject MUST take a string
   /// as parameter.
   public SimpleUObject (String s) {
     super (s); /// Call the constructor of UObject
   }

}

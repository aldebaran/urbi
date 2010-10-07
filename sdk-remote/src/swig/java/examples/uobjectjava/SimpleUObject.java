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

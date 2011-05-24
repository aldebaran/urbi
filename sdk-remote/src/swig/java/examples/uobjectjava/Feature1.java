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

import urbi.*;
import urbi.*;

// ----------------- //
// UObject: Feature1 //
// ----------------- //

/// In this UObject we show how to construct a simple UObject
/// with a constructor dedicated to urbiscript.
/// We also present how to bind functions of this UObject,
/// to make them callable from urbiscript. For this purpose
/// we use:
///
///  * UBindFunction
///
/// When you test it from urbiscript it gives:
///
///    // The prototype of this UObject is avalaible:
///    Feature1;
///    // [00107676] Feature1
///    // You can instanciate new objects from prototype:
///    var f = Feature1.new();
///    // [00118025] Feature1 constructor called !
///    // [00118040] object_11
///    f.testFunction1([1,2,3]);
///    // [00177871] testFunction ([1, 2, 3]) called.
///    // [00177917] "you can return any UValue !"
///    f.testFunction2(1,2);
///    // [00212377] testFunction (1, 2) called.
///    // [00212401] 0
///    f.testFunction3();
///    // [00290254] 42
///
///
/// First note that all Java UObject must extends
/// 'urbi.UObject'
public class Feature1 extends UObject
{
    /// Register your UObject (so that urbi knows about it)
    static { UStart(Feature1.class); };

    // -------------------- //
    // Feature1 constructor //
    // -------------------- //

    public Feature1 (String str) {
        /// First call the super class constructor (mandatory)
        super (str);
        /// Bind the function 'init'.  In Urbi, the init function if
        /// present is considerated as the urbiscript constructor of
        /// the UObject.  Here we choose a constructor with no
        /// arguments.
        UBindFunction (this, "init");
    }

    // --------------------------------------- //
    // Urbiscript constructor for this UObject //
    // --------------------------------------- //

    public int init () {
        myUrbiEcho("Feature1 constructor called !");

        // ----------------- //
        // Functions binding //
        // ----------------- //

        /// Register the function "testFunction1" in this object, so
        /// that it is available in Urbiscript.
        UBindFunction (this, "testFunction1");
        /// NB: The functions you can bind must follow these rules:
        ///   1) They must have between 0 and 16 arguments.
        ///   2) Their arguments can be of type:
	///     urbi.UValue, urbi.UVar, urbi.UList, urbi.UBinary,
	///     urbi.UImage, urbi.USound, urbi.UDictionary,
	///     java.lang.String, java.lang.Integer,
	///     java.lang.Boolean, java.lang.Double, java.lang.Float,
	///     java.lang.Long, java.lang.Short, java.lang.Character,
	///     java.lang.Byte, int, boolean, byte, char, short, long,
	///     float, double
        ///   3) Their return type must be one of the following type:
	///     void, urbi.UValue, urbi.UVar, urbi.UList,
	///     urbi.UBinary, urbi.UImage, urbi.USound,
	///     urbi.UDictionary, java.lang.String, int, boolean,
	///     byte, char, short, long, float, double
        /// You can specify the arguments if there is more than one
        /// function with the same name in your UObject:
        String[] parameters = { "int",
                                "double" };
        UBindFunction (this, "testFunction2", parameters);

        /// You can also bind functions that are in other class, and
        /// they will appear as if they were in this UObject in Urbiscript.
        //SomeClass c = new SomeClass ();
        //UBindFunction (c, "someFunction");

	/// You don't have to pass this if you bind functions of this object.
        UBindFunction ("testFunction3");

        return 0;
    }

    // ----------------------------------- //
    // Binded functions with UBindFunction //
    // ----------------------------------- //


    public UValue testFunction1 (UList i) {

        myUrbiEcho ("testFunction (" + i + ") called.");
        return new UValue ("you can return any UValue !");
    }

    public int testFunction2 (int i, double j) {

        myUrbiEcho ("testFunction (" + i + ", " + j + ") called.");
        return 0;
    }

    /// Overloaded testFunction2 to introduce some ambiguity (two
    /// functions with the same name).
    public int testFunction2 () {
        return 0;
    }

    public int testFunction3 () {
        return 42;
    }

    /// Helper function that allow to 'echo' messages on the Urbi
    /// server.
    private void myUrbiEcho (String str) {

        /// If the message to echo contains ", replace them by ' (it
        /// is necessary because the message send to the server is
        /// enclosed in "):
        str = str.replace ('"', '\'');

        /// Send the message to the system lobby (will be displayed in
        /// user lobby) 'UObject.send'
        send ("Lobby.systemLobby.send (\"" + str +  "\");");
    }
}


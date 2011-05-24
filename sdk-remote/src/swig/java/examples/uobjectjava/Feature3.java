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
// UObject: Feature3 //
// ----------------- //

///
/// In this UObject we demonstrate how to define callbacks functions
/// that will be called each time some urbiscript variable you chose
/// to monitor, change. For this purpose we use:
///
///  * UNotifyChange
///
/// When you test it from urbiscript it gives:
///
///    var f = Feature3.new("Global.a");
///    // [00075686] object_12
///    // [00075696] val1 is changed and is equal to 'some string'
///    // [00075738] val2 is changed and is equal to 42
///    f.val1 = "another string";
///    // [00093754] "another string"
///    // [00093755] val1 is changed and is equal to 'another string'
///    f.val2 = 6;
///    // [00105870] 6
///    // [00105872] val2 is changed and is equal to 6
///    Global.a = -1;
///    // [00117454] -1
///    // [00117457] Global.a is changed and is equal to -1
///
/// First note that all Java UObject must extends
/// 'urbi.UObject'
public class Feature3 extends UObject
{
    /// Register your UObject (so that urbi knows about it)
    static { UStart(Feature3.class); };

    private UVar val1 = new UVar ();
    private UVar val2 = new UVar ();

    // -------------------- //
    // Feature3 constructor //
    // -------------------- //

    public Feature3 (String str) {
        super (str);
        UBindFunction ("init");
    }

    // --------------------------------------- //
    // Urbiscript constructor for this UObject //
    // --------------------------------------- //

    public int init (String somevar_name) {

        UBindVar (val1, "val1");
        val1.setValue ("some string");
        UBindVar (val2, "val2");
        val2.setValue (42);

        // --------------------- //
        // Notify Change binding //
        // --------------------- //

        /// Register the function 'val1Change' of this UObject with
        /// UNotifyChange. Now, each time the value of 'val1' change
        /// in Urbiscript, the 'val1Change' function will be called.
        UNotifyChange (val1, "val1Change");
        ///
        /// NB1: You cannot register any functions with UNotifyChange,
        /// the signature of the function must be one of the
        /// following:
        ///
        ///    [int|void] functionName ()
        /// or
        ///    [int|void] functionName (UVar v)
	///
	///  or any of
	///    [int|void] functionName (<TYPE> v)
	///
	///  with <TYPE> in
	///     urbi.UValue, urbi.UVar, urbi.UList, urbi.UBinary,
	///     urbi.UImage, urbi.USound, urbi.UDictionary,
	///     java.lang.String, java.lang.Integer,
	///     java.lang.Boolean, java.lang.Double, java.lang.Float,
	///     java.lang.Long, java.lang.Short, java.lang.Character,
	///     java.lang.Byte, int, boolean, byte, char, short, long,
	///     float, double
	///
        ///
        /// if you choose the second prototype, then the UVar you
        /// registered will be given as parameter to your callback
        /// function.
	/// if you choose the third, then your callback will be given
	/// the value of the UVar you registered cast'ed to the desired
	/// type.
        ///
        /// NB2: here, the functions we register are unique (there is
        /// only one function named "val1Change" in this UObject, so
        /// we don't need to specify the arguments. If it was not
        /// unique, we could have precised the parameters as follow:
        ///
        ///  String[] parameters = { "urbi.UVar" };
        ///  UNotifyChange (val1, "val1Change", parameters);
        ///

        /// idem.
        UNotifyChange (val2, "val2Change");


        /// With UNotifyChange, you can also be notified when variable
        /// extern to your UObject are changed.  The constructor of
        /// this UObject takes one argument, an UValue. If the user
        /// gives a string with the name of an existing Urbiscript variable,
        /// we bind it to the function "somevarChange"
        if (somevar_name.compareTo("") != 0) {

            /// Then each time the variable named "somevar_name" will changed,
            /// our function somevarChange will be called with the
            /// UVar named "somevar_name" given as parameter
            UNotifyChange (somevar_name, "somevarChange");
        }
	return 0;
    }

    // ------------------------------------------------- //
    // Binded notify change functions with UNotifyChange //
    // ------------------------------------------------- //

    /// Called each time the value of the 'val1' variable change
    public int val1Change (UVar v) {

        /// Ask the Urbi server to echo the following message:
        myUrbiEcho ("val1 is changed and is equal to " + v.getUValue ());
        return 0;
    }

    /// Called each time the value of the 'val2' variable change
    public int val2Change () {

        /// Ask the Urbi server to echo the following message:
        myUrbiEcho ("val2 is changed and is equal to "+ val2.getUValue ());
        return 0;
    }

    public int somevarChange (UVar v) {

        /// Ask the Urbi server to echo the following message:
        myUrbiEcho (v.getName () + " is changed and is equal to "
                    + v.getUValue ());
        return 0;
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


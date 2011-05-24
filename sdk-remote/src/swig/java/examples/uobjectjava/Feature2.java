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
// UObject: Feature2 //
// ----------------- //

/// In this UObject we show how to make variables member of this
/// UObject appear in urbiscript. For this purpose we use:
///
///  * UBindVar
///
/// When you test it from urbiscript it gives:
///
///    var f = Feature2.new();
///    // [00005575] object_12
///    (f.localSlotNames - Feature2.localSlotNames).sort;
///    // [00015742] ["asobject_12", "val1", "val1_quote_n", "val2",
///    // "val2_quote_n"]
///    f.val1;
///    // [00019710] "some string"
///    f.val2;
///    // [00026558] 42
///
/// NB: several other variable have appeared, these are system
/// variable that you must not use.
///
///
/// First note that all Java UObject must extends
/// 'urbi.UObject'
public class Feature2 extends UObject
{
    /// Register your UObject (so that urbi knows about it)
    static { UStart(Feature2.class); };

    /// Binded UVar val1
    private UVar val1 = new UVar ();

    /// Binded UVar val2
    private UVar val2 = new UVar ();

    // -------------------- //
    // Feature2 constructor //
    // -------------------- //

    public Feature2 (String str) {
        super (str);
        UBindFunction ("init");
    }

    // --------------------------------------- //
    // Urbiscript constructor for this UObject //
    // --------------------------------------- //

    public int init () {
        // ----------------- //
        // Variables Binding //
        // ----------------- //

        /// Bind the val1 UVar, so that it appears in urbiscript.
        ///
        /// The prototype of UBindVar is:
        ///      UBindVar (UVar v, String name)
        ///
        UBindVar (val1, "val1");
        /// Assign the value "some string" to val1.  NB: now the type
        /// of the UVar is String.
        val1.setValue ("some string");

        /// Bind the val2 UVar, so that it appears in urbiscript.
        UBindVar (val2, "val2");
        /// Assign the value 42 to val2 NB: now the type of the UVar
        /// is UFloat (float).
        val2.setValue (42);

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

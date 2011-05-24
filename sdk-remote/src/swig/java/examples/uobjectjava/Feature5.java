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
// UObject: Feature5 //
// ----------------- //

///
/// In this UObject we demonstrate:
///
///  * UVar setting and getting
///
/// When you test it from urbiscript it gives:
///
///     var f = Feature5.new("");
///     // [00026724] object_14
///     var Global.a = 42;
///     // [00030984] 42
///     f.getValue("Global.a");
///     // [00043315] Global.a equals 42
///     // [00043357] 0
///     f.setValue("Global.a", 41);
///     Global.a;
///     // [00073470] 41
///
/// First note that all Java UObject must extends
/// 'urbi.UObject'
public class Feature5 extends UObject
{
    /// Register your UObject (so that urbi knows about it)
    static { UStart(Feature5.class); };

    // -------------------- //
    // Feature5 constructor //
    // -------------------- //

    public Feature5 (String str) {
	super (str);
	UBindFunction (this, "init");
    }

    // --------------------------------------- //
    // Urbiscript constructor for this UObject //
    // --------------------------------------- //

    public int init (UValue somevar_name) {

	/// These function are used to demonstrate the setting and the
	/// getting of an UVar extern to this UObject.  Please see
	/// comments on theses functions below.
	UBindFunction (this, "getValue");
	UBindFunction (this, "setValue");
	return 0;
    }


    // ------------------------------------------------------------------ //
    // Test getting & setting the value of an UVar extern to this UObject //
    // ------------------------------------------------------------------ //

    /// WARNING: Here we demonstrate the use of synchronous retrieving
    /// of UVar values.  This feature exist, so we explain it, in case
    /// you _really_ need it, but try not to use it.  Prefer the use
    /// of UNotifyOnRequest which is asynchronous (ie will not freeze
    /// your UObject if the Urbi server doesn't answer).
    ///
    public int getValue (String name) {

	/// Create an UVar with the name of an existing UVar, and
	/// retrieve its value synchronously:
	UVar tmp = new UVar (name);
	tmp.syncValue(); /// ! WARNING ! This call can freeze your
			 /// UObject if the server does not answer.
	myUrbiEcho (name + " equals " + tmp.getUValue ());

	return 0;
    }

    /// Here we demonstrate the setting of an UVar external to this
    /// UObject
    public void setValue (String name, UValue value) {

	/// Create an UVar with the name of an existing UVar
	UVar tmp = new UVar (name);
	/// And assign a value to it. The walue will be set on the
	/// UVar existing on the server. All is transparent.
	tmp.setValue(value);
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


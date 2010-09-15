package examples.uobjectjava;

import urbi.*;
import urbi.generated.*;

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
/// 'liburbi.main.UObjectJava'
public class Feature5 extends UObjectJava
{

    // -------------------- //
    // Feature5 constructor //
    // -------------------- //

    public Feature5 (String str)
	throws Exception {
	super (str);
	UBindFunction (this, "init");
    }

    // --------------------------------------- //
    // Urbiscript constructor for this UObject //
    // --------------------------------------- //

    public int init (UValue somevar_name)
	throws Exception {

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
    public int getValue (UValue varname) {

	/// Create an UVar with the name of an existing UVar, and
	/// retrieve its value synchronously:
	String name = varname.getString ();
	UVar tmp = new UVar (name);
	tmp.syncValue(); /// ! WARNING ! This call can freeze your
			 /// UObject if the server does not answer.
	myUrbiEcho (name + " equals " + tmp.getUValue ());

	return 0;
    }

    /// Here we demonstrate the setting of an UVar external to this
    /// UObject
    public void setValue (UValue name, UValue value) {

	/// Create an UVar with the name of an existing UVar
	UVar tmp = new UVar (name.getString ());
	/// And assign a value to it. The walue will be set on the
	/// UVar existing on the server. All is transparent.
	tmp.set(value);
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


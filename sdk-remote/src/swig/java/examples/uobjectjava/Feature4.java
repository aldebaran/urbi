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

// ----------------- //
// UObject: Feature4 //
// ----------------- //

///
/// In this UObject we demonstrate the use of timers mechanisms,
/// which allows you to define callbacks functions which will be
/// called at some frequency you defined. For this purpose we use:
///
///  * USetUpdate
///  * USetTimer
///
/// When you test it from urbiscript it gives:
///
///     Feature4.new;
///     // [00005826] Update called 1 time.
///     // [00005826] myTimerFunction called 1 time.
///     // [00005833] object_13
///     // [00005991] myTimerFunction called 2 times.
///     // [00006194] myTimerFunction called 3 times.
///     // [00006290] Update called 2 times.
///     // [00006399] myTimerFunction called 4 times.
///     // [00006602] myTimerFunction called 5 times.
///     // [00006794] Update called 3 times.
///     // [00007298] Update called 4 times.
///     // [00007802] Update called 5 times.
///
/// First note that all Java UObject must extends
/// 'urbi.UObject'
public class Feature4 extends UObject
{
    /// Register your UObject (so that urbi knows about it)
    static { UStart(Feature4.class); };


    /// Some variables used in the functions below
    private int timer_count = 1;
    private int update_count = 1;

    // -------------------- //
    // Feature4 constructor //
    // -------------------- //

    public Feature4 (String str) {
	super (str);
	UBindFunction ("init");
    }

    // --------------------------------------- //
    // Urbiscript constructor for this UObject //
    // --------------------------------------- //

    public int init () {

	// --------------- //
	// Timer functions //
	// --------------- //

	/// Call the update function every 500 ms
	USetUpdate (500);
	/// Indeed, you can overload the function
	///
	///    int update ()
	///
	/// That is defined in the class UObject, and set a timer with
	///
	///    void USetUpdate (double period)
	///
	/// (period is in milliseconds).  Then every 'period' time
	/// your update function will be called.
	///
	/// USetTimer (double period, Object o, String functionName)
	/// allows you to set a callback function that will be called
	/// every 'period' milliseconds.  Here we set that the
	/// "myTimerFunction" will be called each 200 ms.
	USetTimer (200, this, "myTimerFunction");
	/// NB: you can also specify the arguments of the callback
	/// function with USetTimer (double period, object o, String
	/// functionName, String[] args) like in UBindFunction
	return 0;
    }


    // ------------------------------------------------ //
    // Timer function set with USetUpdate and USetTimer //
    // ------------------------------------------------ //

    public int update () {

	/// We display the update message 5 times
	if (update_count <= 5) {
	    String s = "";
	    if (update_count > 1)
		s = "s";
	    myUrbiEcho ("Update called " + update_count + " time" + s + ".");
	    update_count++;
	}
	return 0;
    }


    public int myTimerFunction () {

	/// We display the myTimerFunction  message 5 times
	if (timer_count <= 5) {
	    String s = "";
	    if (timer_count > 1)
		s = "s";
	    myUrbiEcho ("myTimerFunction called " + timer_count
			+ " time" + s + ".");
	    timer_count++;
	}
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


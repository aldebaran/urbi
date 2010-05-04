package examples.uobjectjava;

import liburbi.main.*;


/// Some class used in one of the example.
class SomeClass
{
    /// Some function.
    public UValue someFunction ()
    {
	return new UValue ("someFunction () called.");
    }
}


// ---------------- //
// UObject: Feature //
// ---------------- //

///
/// In this UObject we demonstrate the use of some of the features availables in
/// the java UObjects:
///
///  * UBindVar
///  * UBindFunction
///  * UNotifyAccess
///  * UNotifyOnRequest
///  * USetUpdate
///  * USetTimer
///  * UVar setting and getting
///
/// We tried to follow the API of C++ UObjects (so if you already know them, the
/// use of UObjects in java will be straigtforward).
///
/// First note that all Java UObject must extends 'liburbi.main.UObjectJava'
public class Feature extends UObjectJava
{

    /// Binded UVar val1
    private UVar val1 = new UVar ();

    /// Binded UVar val2
    private UVar val2 = new UVar ();

    /// Some variables used in the functions below
    private int timer_count = 1;
    private int update_count = 1;
    private String onRequestVarName;


    // -------------------- //
    // Features constructor //
    // -------------------- //

    public Feature (String str) {

	/// First call the super class constructor (mandatory)
	super (str);

	/// Create a group of this UObject name + 's' (ie: 'Features'), and populate it
	/// with all the UObject of this UObject type.
	/// In urbi it will give:
	///   a = new Feature ("");
	///   b = new Feature ("");
	///   group Features;
	///   [00117759] ["a", "b"]
	///
	/// This call  must be in the UObject constructor if present.
	UAutoGroup ();

	try
	{
	    /// Bind the function 'init'.
	    /// In Urbi, the init function if present is considerated as the constructor
	    /// of the UObject.
	    /// Here we choose a constructor with one argument, an UValue.
	    UBindFunction (this, "init");
	}
	catch (Exception e) {
	    System.out.println (e);
	}
    }



    // --------------------------------- //
    // Urbi constructor for this UObject //
    // --------------------------------- //

    public int init (UValue somevar_name) {

	try
	{
	    // ----------------- //
	    // Variables Binding //
	    // ----------------- //

	    /// Assign the value "some string" to val1.
	    /// NB: now the type of the UVar is String.
	    val1.set ("some string");
	    /// Bind the val1 UVar, so that it appears in Urbi.
	    ///
	    /// The prototype of UBindVar is:
	    ///      UBindVar (UVar v, String name)
	    ///
	    UBindVar (val1, "val1");

	    /// Assign the value 42 to val2
	    /// NB: now the type of the UVar is UFloat (float).
	    val2.set (42);
	    /// Bind the val2 UVar, so that it appears in Urbi.
	    UBindVar (val2, "val2");


	    /// Now in urbi the UObject Feature contains two
	    /// member variable (plus the 'load' variable, present by
	    /// default).
	    /// ex:
	    ///
	    /// c = new Feature ("");
	    /// c,
	    /// [01293791] OBJ [load:1,val1:"some string",val2:42]



	    // --------------------- //
	    // Notify Change binding //
	    // --------------------- //

	    /// Register the function 'val1Change' of this UObject with
	    /// UNotifyChange. Now, each time the value of 'val1' change in
	    /// Urbi, the 'val1Change' function will be called.
	    UNotifyChange (val1, "val1Change");
	    ///
	    /// NB1: You cannot register any functions with UNotifyChange, the
	    /// signature of the function must be one of the following:
	    ///
	    ///    int functionName ()
	    /// or
	    ///    int functionName (UVar v)
	    ///
	    /// if you choose the second prototype, then the UVar you registered
	    /// will be given as parameter to your callback function.
	    ///
	    /// NB2: here, the functions we register are unique (there is only one
	    /// function named "val1Change" in this UObject, so we don't need to specify
	    /// the arguments. If it was not unique, we could have precised the parameters
	    /// as follow:
	    ///
	    ///  String[] parameters = { "liburbi.main.UVar" };
	    ///  UNotifyChange (val1, "val1Change", parameters);
	    ///
	    /// NB3: If you change the type of the UVar, the Notify Change callbacks will
	    /// be removed.
	    /// ex: here 'val1' is of type String. If in Urbi I type:
	    ///
	    /// c = new Feature ();
	    /// c,
	    /// [01293791] OBJ [load:1,val1:"some string",val2:42]
	    /// c.val1 = 42; /// the type becomes UFloat
	    ///
	    /// Then the 'val1Change' function won't be triggered anymore.


	    /// idem.
	    UNotifyChange (val2, "val2Change");


	    /// With UNotifyChange, you can also be notified when variable extern to your
	    /// UObject are changed.
	    /// The constructor of this UObject takes one argument, an UValue. If the
	    /// user gives a string with the name of an existing Urbi variable, we bind it
	    /// to the function "somevarChange"
	    String name = somevar_name.getString ();
	    if (name.compareTo("") != 0) {

		/// Then each time the variable named "name" will changed, our function
		/// somevarChange will be called with the UVar named "name" given as
		/// parameter
		UNotifyChange (name, "somevarChange");

		/// ex:
		///
		/// var myvar = 42;
		/// connectionID; /// * See below
		/// [04823100] "U135737384"
		/// d = new Feature ("U135737384.myvar");
		/// myvar = 43;
		/// [04859053] *** U135737384.myvar is changed and is equal to 43


		/// * In URBI kernel 1.x, variable names are always of the form
		/// prefix.suffix and when no prefix is supplied, a prefix local to
		/// the connection is silently added so that x in one connection will
		/// not interfere with x in another connection.
		/// For example, when you type x URBI will in fact use U596851624.x
		/// in its memory, with U596851624 being the identifier of your current
		/// connection (where you typed x in). In the same way, function calls
		/// have a local namespace attributed, so that you can do recursive
		/// function calls without interferences. This will be redesigned in
		/// URBI 2.0 with advanced name resolution and name space support.
	    }


	    // ----------------- //
	    // Functions binding //
	    // ----------------- //

	    /// Register the function "testFunction1" in this object, so that it is
	    /// available in Urbi.
	    UBindFunction (this, "testFunction1");
	    /// NB: The functions you can bind must follow these rules:
	    ///   1) They must have between 0 and 16 arguments.
	    ///   2) All their arguments must be of type liburbi.main.UValue
	    ///   3) Their return type must be one of the following type:
	    ///   void, boolean, byte, char, short, int, long, float, double, UValue

	    /// You can specify the arguments if there is more than one function with
	    /// the same name in your UObject:
	    String[] parameters = { "liburbi.main.UValue", "liburbi.main.UValue" };
	    UBindFunction (this, "testFunction2", parameters);

	    /// You can also bind functions that are in other class, and they will appear
	    /// as if they were in this UObject in Urbi.
	    SomeClass c = new SomeClass ();
	    UBindFunction (c, "someFunction");

	    UBindFunction (this, "testFunction3");

	    /// ex:
	    ///
	    /// e = new Feature ("");
	    ///
	    /// e.testFunction1 ([1,2,3]);
	    /// [07527967] "you can return any UValue !"
	    /// [07527989] *** testFunction ([1, 2, 3]) called.
	    ///
	    /// e.testFunction2 (1,2),
	    /// [07544191] *** testFunction (1, 2) called.
	    /// [07544207] 0
	    ///
	    /// e.someFunction (),
	    /// [07561476] "someFunction () called."


	    /// These function are used to demonstrate the setting and the getting
	    /// of an UVar extern to this UObject.
	    /// Please see comments on theses functions below.
	    UBindFunction (this, "getValue");
	    UBindFunction (this, "setValue");


	    /// These function are used to demonstrate the use of UNotifyOnRequest
	    /// Please see comments on theses functions below.
	    UBindFunction (this, "setNotifyOnRequest");
	    UBindFunction (this, "requestValue");


	    // --------------- //
	    // Timer functions //
	    // --------------- //

	    /// Call the update function every 500 ms
	    USetUpdate (500);
	    /// Indeed, you can overload the function
	    ///
	    ///    int update ()
	    ///
	    /// That is defined in the class UObjectJava, and set a timer with
	    ///
	    ///    void USetUpdate (double period)
	    ///
	    /// (period is in milliseconds).
	    /// Then every 'period' time your update function will be called.


	    ///
	    /// USetTimer (double period, Object o, String functionName)
	    /// allows you to set a callback function that will be called
	    /// every 'period' milliseconds.
	    /// Here we set that the "myTimerFunction" will be called each
	    /// 200 ms.
	    USetTimer (200, this, "myTimerFunction");
	    /// NB: you can also specify the arguments of the callback function
	    /// with
	    ///   USetTimer (double period, object o, String functionName, String[] args)
	    /// like in UBindFunction

	    /// ex:
	    /// f = new Feature ("");
	    /// [10044160] *** Update called 1 time.
	    /// [10044172] *** myTimerFunction called 1 time.
	    /// [10044324] *** myTimerFunction called 2 times.
	    /// [10044524] *** myTimerFunction called 3 times.
	    /// [10044616] *** Update called 2 times.
	    /// [10044728] *** myTimerFunction called 4 times.
	    /// [10044932] *** myTimerFunction called 5 times.
	    /// [10045116] *** Update called 3 times.
	    /// [10045624] *** Update called 4 times.
	    /// [10046132] *** Update called 5 times.


	}
	catch (Exception e) {
	    System.out.println (e);
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



    // ----------------------------------- //
    // Binded functions with UBindFunction //
    // ----------------------------------- //


    public UValue testFunction1 (UValue i) {

	myUrbiEcho ("testFunction (" + i + ") called.");
	return new UValue ("you can return any UValue !");
    }

    public int testFunction2 (UValue i, UValue j) {

	myUrbiEcho ("testFunction (" + i + ", " + j + ") called.");
	return 0;
    }

    /// Overloaded testFunction2 to introduce some ambiguity (two functions
    /// with the same name).
    public int testFunction2 () {
	return 0;
    }

    public int testFunction3 () {
	myUrbiEcho ("int myUObject::testFunction3");
	return 42;
    }


    // ------------------------------------------------------------------ //
    // Test getting & setting the value of an UVar extern to this UObject //
    // ------------------------------------------------------------------ //


    /// WARNING:
    /// Here we demonstrate the use of synchronous retrieving of UVar values.
    /// This feature exist, so we explain it, in case you _really_ need it,
    /// but try not to use it.
    /// Prefer the use of UNotifyOnRequest which is asynchronous (ie will not
    /// freeze your UObject if the Urbi server doesn't answer).
    ///
    public int getValue (UValue varname) {

	/// Create an UVar with the name of an existing UVar, and retrieve
	/// its value synchronously:
	String name = varname.getString ();
	UVar tmp = new UVar (name);
	tmp.syncValue(); /// ! WARNING ! This call can freeze your UObject if
	                 /// the server does not answer.
	myUrbiEcho (name + " equals " + tmp.getUValue ());

	/// ex:
	///
	/// g = new Feature ("");
	/// var myvar = 42,
	/// connectionID,
	/// [10777412] "U135737384"
	/// g.getValue ("U135737384.myvar");
	/// [10793237] *** U135737384.myvar equals 42

	return 0;
    }

    /// Here we demonstrate the setting of an UVar external to this UObject
    public void setValue (UValue name, UValue value) {

	/// Create an UVar with the name of an existing UVar
	UVar tmp = new UVar (name.getString ());
	/// And assign a value to it. The walue will be set on the UVar existing
	/// on the server. All is transparent.
	tmp.set(value);

	/// ex:
	///
	/// g = new Feature ("");
	/// var myvar = 42,
	/// connectionID,
	/// [10777412] "U135737384"
	/// g.setValue ("U135737384.myvar", 43);
	/// myvar,
	/// [11212517] 43
    }


    // ----------------------------- //
    // Test UNotifyOnRequest feature //
    // ----------------------------- //

    /// the UNotifyONRequest feature is similar to the UNotifyChange
    /// feature, except that the binded function is not called each
    /// time the value of the variable change. You have to request
    /// explicitly that the function be called in order for the function
    /// to be called.
    /// This feature is used to retrieve asynchronously the value of a
    /// variable.
    /// The example we give here is only an example of the feature
    /// you might not use it like that in real use cases.

    /// Give the name of an existing UVar as parameter. It will bind
    /// the function "onRequestChange" on this UVar with UNotifyONRequest
    public void setNotifyOnRequest (UValue name) {
	try
	{
	    onRequestVarName = name.getString ();
	    UNotifyOnRequest (onRequestVarName, "onRequestChange");
	}
	catch (Exception e) {
	    System.out.println (e);
	}
    }

    /// Request that the function "onRequestChange" be called.
    public void requestValue ()
    {
	UVar v = new UVar (onRequestVarName);
	/// We want to know the value of the UVar, but we don't want to
	/// freeze this UObject if the server answer slowly.
	/// So we call the UVar function "requestValue ()". This will trigger
	/// all the functions binded to this UVar with UNotifyONRequest
	v.requestValue ();
    }

    /// This function will be called when the value of the UVar is available.
    /// The UVar is given as parameter to this function.
    public int onRequestChange (UVar v) {
	myUrbiEcho (v.getName () + " is equal to " + v.getUValue ());
	return 0;
    }

    /// ex:
    ///
    /// g = new Feature ("");
    /// var myvar = 42,
    /// connectionID,
    /// [10777412] "U135737384"
    /// g.setNotifyOnRequest ("U135737384.myvar");
    /// g.requestValue (),
    /// [12355188] *** U135737384.myvar is equal to 42



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
	    myUrbiEcho ("myTimerFunction called " + timer_count + " time" + s + ".");
	    timer_count++;
	}
	return 0;
    }




    /// Helper function that allow to 'echo' messages on the Urbi server.
    private void myUrbiEcho (String str) {

	/// If the message to echo contains ", replace them by ' (it is
	/// necessary because the message send to the server is enclosed in "):
	str = str.replace ('"', '\'');

	/// Send the 'echo' command to the Urbi server with the static function
	/// 'UObject.send'
	send ("+connection(\"all\"):echo (\"" + str +  "\");");
    }

}


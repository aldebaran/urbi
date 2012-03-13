/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

package tests.all;

import java.util.Locale;
import java.util.Map;
import urbi.Log;
import urbi.UBinary;
import urbi.UBinaryType;
import urbi.UDataType;
import urbi.UDictionary;
import urbi.UEvent;
import urbi.UImage;
import urbi.UImageFormat;
import urbi.UList;
import urbi.UMatrix;
import urbi.UObject;
import urbi.UProperty;
import urbi.USound;
import urbi.USoundFormat;
import urbi.USoundSampleFormat;
import urbi.UValue;
import urbi.UValueVector;
import urbi.UVar;
import urbi.UVector;

// TODO:
// - changer les getters
// UValueValue()
// doubleValue()
// ...
//
// - UStart with just the class
// - UBindFunction avec du typage statique
// - verifier http://www.swig.org/Doc1.3/Java.html#memory_management
// - http://svn.osgeo.org/gdal/trunk/gdal/swig/include/java/ogr_java.i
//   https://valelab.ucsf.edu/svn/micromanager2/trunk/MMCoreJ_wrap/MMCoreJ.i
//
// - more thorough testing of UDictionary
//
// - checker pkoi dans list ils font new UValue(val) plutot que val
//   tout court dans all.cc
//
// - stop checking the types when calling UBind'ed functions,
//   but rather at setting (putting point on fct)
//
// - uniformiser UBindFunction et UBindVar ?
//
// - generer javadoc

// DONE.
// - changer la hierarchie pour avoir tout dans urbi.*

public class All extends UObject
{
    static
    {
	UStartRename(All.class, "remall");
	UStartRename(All.class, "remall2");
    }

    public All (String s)
    {
	super(s);

	count = 0;

	//if (getenv("CTOR_EXCEPTION") &&
	//    !strcmp(getenv("CTOR_EXCEPTION"), "true"))
	//    throw std::runtime_error("constructor failure");

	UBindFunction("init");
	//UBindFunction(all, setOwned);
	UBindFunction("setNotifyChange");

	///** BYPASS check **/
	UBindFunction("setBypassNotifyChangeBinary");
	UBindFunction("setBypassNotifyChangeImage");
	UBindFunctions("markBypass"/*, markRTP*/);
	UBindFunctions("selfWriteB", "selfWriteI"/*, "selfWriteVD"*/);

	UBindFunctions("setNotifyChangeByName", "setNotifyChangeByUVar");
	//UBindFunctions(all, setNotifyAccess, setNotifyChangeByName, setNotifyChangeByUVar, sendEvent8Args, unnotify);
	//UBindFunction(all, read);
	//UBindFunction(all, write);
	UBindFunction("readByName");
	UBindFunction("writeByName");
	UBindFunction("writeByUVar");
	UBindFunction("writeOwnByName");
	UBindFunction("urbiWriteOwnByName");
	UBindFunction("sendString");
	UBindFunction("sendBuf");
	UBindFunction("sendPar");
	UBindFunction("typeOf");
	//UBindFunction(all, uobjectName);
	//UBindFunction(all, allUObjectName);
	UBindVar(a, "a");
	UBindVar(b, "b");
	UBindVar(c, "c");
	UBindVar(d, "d");
	//UBindVars(all, b, c, d);
	UBindVar(initCalled, "initCalled");
	initCalled.setValue(0);
	UBindVar(lastChange, "lastChange");
	UBindVar(lastAccess, "lastAccess");
	UBindVar(lastChangeVal, "lastChangeVal");
	lastChangeVal.setValue(-1);
	UBindVar(lastAccessVal, "lastAccessVal");
	UBindVar(removeNotify, "removeNotify");
	removeNotify.setValue("");
	// Properties.
	UBindFunction("readProps");
	UBindFunction("writeProps");

	UBindFunctions
            ("writeD", "writeS", "writeL", "writeM",
             "writeB", "makeCall", "writeBNone", "writeI", "writeSnd"
             //, "writeRI", "writeRSnd"
             );
	//
	UBindFunctions
            ("transmitD", "transmitS", "transmitL", "transmitM", "transmitB",
             "transmitI", "transmitSnd",
             "transmitVector", "transmitMatrix");
	//UBindFunction(this, "transmitO");

	//UBindFunction(all, loop_yield);
	//UBindFunction(urbi::UContext, side_effect_free_get);
	//UBindFunction(urbi::UContext, side_effect_free_set);
	UBindFunction("yield");

        // We are not hidding the previous version of the function,
        // since the argument types are different.  So we need to
        // specify which flavor we want to bind into urbiscript.
        // Unfortunately, there is no support for urbiscript-name
        // vs. Java-name, so we rather need to specify the list of
        // arguments.  Unfortunately again, UObject.java does not
        // support "double" as argument type name, it needs class
        // names.
        String[] yield_for_args = { "java.lang.Double" };
	UBindFunction(this, "yield_for", yield_for_args);

	UBindFunction("getDestructionCount");

	UBindFunction("invalidRead");
	UBindFunction("invalidWrite");

	UBindEvent(ev, "ev");
	UBindFunction("sendEvent");
	UBindFunction("sendEvent2Args");
	UBindFunction("sendNamedEvent");

	UBindFunction("throwException");
	//UBindFunction(all, socketStats);
	//UBindVars(all, periodicWriteTarget, periodicWriteType, periodicWriteRate,
	//	  changeCount);
        UBindFunctions("setLocale", "getLocale");

	UBindVar(changeCount, "changeCount");
	//UNotifyChange(periodicWriteRate, &all::onRateChange);
	vars[0] = a;
	vars[1] = b;
	vars[2] = c;
	vars[3] = d;
	//
	UBindFunctions("notifyWriteA", "writeAD", "writeAS", "writeAB");
    }


    protected void finalize()
    {
	Log.info("all.finalize");
	++destructionCount;
	super.finalize();
    }

    //
    //    void onRateChange(urbi::UVar&)
    //    {
    //	USetUpdate((ufloat)periodicWriteRate * 1000.0);
    //    }
    //    virtual int update()
    //    {
    //	int target = periodicWriteTarget;
    //	int type = periodicWriteType;
    //	switch(type)
    //	{
    //	    case urbi::DATA_STRING:
    //		*vars[target] = string_cast(libport::utime());
    //		break;
    //	    case urbi::DATA_BINARY:
    //		selfWriteB(target,  string_cast(libport::utime()));
    //		break;
    //	    case urbi::DATA_DOUBLE:
    //	    default:
    //		*vars[target] = libport::utime();
    //		break;
    //	}
    //	return 0;
    //    }
    public void yield_for(double sDuration)
    {
       super.yield_for((int)(sDuration * 1000000.0));
    }
    public int setBypassNotifyChangeBinary(String name)
    {
	UNotifyChange(name, "onBinaryBypass");
	return 0;
    }

    public int setBypassNotifyChangeImage(String name)
    {
	UNotifyChange(name, "onImageBypass");
	return 0;
    }

    public int markBypass(int id, boolean state)
    {
	return vars[id].setBypass(state) ? 1 : 0;
    }

    //    int markRTP(int id, bool state)
    //    {
    //	vars[id]->useRTP(state);
    //	return 0;
    //    }

    //    void unnotify(int id)
    //    {
    //	vars[id]->unnotify();
    //    }

    public int onBinaryBypass(UVar var)
    {
        Log.info("onBinaryBypass: in: "
                 + new String(var.ubinaryValue().getData()));
	UBinary b = var.ubinaryValue();
	byte[] data = b.getData();
	for (int i=0; i<b.getSize(); ++i)
	    data[i]++;
        b.setData(data);
        Log.info("onBinaryBypass: out: "
                 + new String(var.ubinaryValue().getData()));
	return 0;
    }

    public void onImageBypass(UVar var)
    {
        Log.info("onImageBypass: in: "
                 + new String(var.uimageValue().getData()));
	UImage b = var.uimageValue();
	byte[] data = b.getData();
	for (int i=0; i<b.getSize(); ++i)
	    data[i]++;
        b.setData(data);
        Log.info("onImageBypass: out: "
                 + new String(var.uimageValue().getData()));
    }

    public String selfWriteB(int idx, String content)
    {
	UBinary b = new UBinary();
	b.setType(UBinaryType.BINARY_IMAGE);
	// Dup since we want to test no-copy op: the other end will write.
	byte[] array = content.getBytes();
	byte[] array2 = new byte[array.length];
	System.arraycopy(array, 0, array2, 0, array.length);
	b.setData(array2);
	vars[idx].setValue(b);
	String res = new String(b.getData());
	// FIXME: free b.data array ?
	return res;
    }

    public String selfWriteI(int idx, String content)
    {
	UImage i = new UImage();
	byte[] array = content.getBytes();
	byte[] array2 = new byte[array.length];
	System.arraycopy(array, 0, array2, 0, array.length);
	i.setData(array2);
	vars[idx].setValue(i);
	String res = new String(i.getData());
	// FIXME: free i.data array ?
	return res;
    }

    public int writeOwnByName(String name, int val)
    {
	UVar v = new UVar(get__name() + "." + name);
	v.setValue(val);
	return 0;
    }

    public int urbiWriteOwnByName(String name, int val)
    {
	send(String.format("%s.%s = %d;", get__name(), name, val));
	return 0;
    }

    //    void selfWriteVD(int i, std::vector<double> v)
    //    {
    //	*vars[i] = v;
    //    }

    public String typeOf(String name)
    {
	UVar v = new UVar(name);
	v.syncValue();
	return v.getUValue().format_string();
    }

    public int init(int fail)
    {
	initCalled.setValue(1);
	if (fail > 1)
	    throw new RuntimeException("KABOOOM");
	return fail;
    }

    //    int setOwned(int id)
    //    {
    //	threadCheck();
    //	UOwned(*vars[id]);
    //	return 0;
    //    }

    public int setNotifyChange(int id)
    {
	UNotifyChange(vars[id], "onChange");
	return 0;
    }

    public int setNotifyChangeByUVar(UVar v)
    {
	UNotifyChange(v, "onChange");
	return 0;
    }

    //    int setNotifyAccess(int id)
    //    {
    //	threadCheck();
    //	UNotifyAccess(*vars[id], &all::onAccess);
    //	return 0;
    //    }

    public int setNotifyChangeByName(String name)
    {
	UNotifyChange(name, "onChange");
	return 0;
    }

    //    int read(int id)
    //    {
    //	threadCheck();
    //	int v = *vars[id];
    //	return v;
    //    }
    //    int write(int id, int val)
    //    {
    //	threadCheck();
    //	*vars[id] = val;
    //	return val;
    //    }

    public void invalidWrite()
    {
	UVar v = new UVar();
	v.setValue(12);
    }

    public void invalidRead()
    {
	UVar v = new UVar();
	int i = v.intValue();
    }

    public int readByName(String name)
    {
	UVar v = new UVar(name);
	v.syncValue();
	return v.intValue();
    }

    public int writeByName(String name, int val)
    {
	UVar v = new UVar(name);
	v.setValue(val);
	return val;
    }

    public int writeByUVar(UVar v, UValue val)
    {
	v.setValue(val);
	return 0;
    }

    public int onChange(UVar v)
    {
	lastChange.setValue(v.getName());
	changeCount.setValue(++count);
	if (v.type() == UDataType.DATA_DOUBLE)
	    {
		int val = v.intValue();
		lastChangeVal.setValue(val);
	    }
	if (removeNotify.stringValue().equals(v.getName()))
	    {
		v.unnotify();
		removeNotify.setValue("");
	    }
	return 0;
    }

    //    int onAccess(urbi::UVar& v)
    //    {
    //	threadCheck();
    //	static int val = 0;
    //	lastAccess = v.get_name();
    //	val++;
    //	v = val;
    //	lastAccessVal = val;
    //	if ((std::string)removeNotify == v.get_name())
    //	{
    //	    v.unnotify();
    //	    removeNotify = "";
    //	}
    //	return 0;
    //    }

    public void sendEvent()
    {
	ev.emit();
    }

    public void sendEvent2Args(UValue v1, UValue v2)
    {
	ev.emit(v1, v2);
    }

    // void sendEvent8Args()
    // {
    //	ev.emit(0, "foo", 5.1, 4, 5, 6, 7, 8);
    // }

    public void sendNamedEvent(String name)
    {
	UEvent tempEv = new UEvent(name);
	tempEv.emit();
    }

    /// Return the value of the properties of the variable \a name.
    public UList readProps(String name)
    {
	UVar v = new UVar(name);
	UList res = new UList();
	res.push_back(v.getProp(UProperty.PROP_RANGEMIN).doubleValue());
	res.push_back(v.getProp(UProperty.PROP_RANGEMAX).doubleValue());
	res.push_back(v.getProp(UProperty.PROP_SPEEDMIN).doubleValue());
	res.push_back(v.getProp(UProperty.PROP_SPEEDMAX).doubleValue());
	res.push_back(v.getProp(UProperty.PROP_DELTA).doubleValue());
	res.push_back(v.getProp(UProperty.PROP_BLEND));
	res.push_back(v.getProp(UProperty.PROP_CONSTANT));
	Log.info("all.readProps: " + res.toString());
	return res;
    }

    public int writeProps(String name, double val)
    {
	UVar v = new UVar(name);
	v.setProp(UProperty.PROP_RANGEMIN, val);
	v.setProp(UProperty.PROP_RANGEMAX, val);
	v.setProp(UProperty.PROP_SPEEDMIN, val);
	v.setProp(UProperty.PROP_SPEEDMAX, val);
	v.setProp(UProperty.PROP_DELTA, val);
	v.setProp(UProperty.PROP_BLEND, val);
	v.setProp(UProperty.PROP_CONSTANT, val);
	return 0;
    }


    /**  Test write to UVAR.  **/

    public int writeD(String name, double val)
    {
	Log.info("writeD " + name);
	UVar v = new UVar (name);
	v.setValue(val);
	return 0;
    }

    public int writeS(String name, String val)
    {
	Log.info("writeS " + name);
	UVar v = new UVar(name);
	v.setValue(val);
	return 0;
    }

    public int writeL(String name, String val)
    {
	Log.info(String.format("writeL %s", name));
	UVar v = new UVar(name);
	UList l = new UList();
	l.push_back(val);
	l.push_back(42);
	v.setValue(l);
	return 0;
    }

    public int writeM(String name, String val)
    {
	Log.info(String.format("writeM %s", name));
	UVar v = new UVar(name);
	UDictionary d = new UDictionary();
	d.put(val, 42);
	d.put("foo", new UList());
	v.setValue(d);
	return 0;
    }

    public int writeB(String name, String content)
    {
	UVar v = new UVar(name);
	UBinary val = new UBinary();
	val.setType(UBinaryType.BINARY_UNKNOWN);
	val.setData(content.getBytes());
	v.setValue(val);
	return 0;
    }

    public int writeBNone(String name, String content)
    {
	UVar v = new UVar(name);
	UBinary val = new UBinary();
	val.setData(content.getBytes());
	v.setValue(val);
	return 0;
    }

    public int writeI(String name, String content)
    {
	UVar v = new UVar(name);
	UImage i = new UImage();
	i.setImageFormat(UImageFormat.IMAGE_JPEG);
	i.setWidth(42);
	i.setHeight(42);
	i.setData(content.getBytes());
	v.setValue(i);
	i.deleteData();	// WARNING: this function is used only when data was allocated JAVA side
	return 0;
    }

    public int writeSnd(String name, String content)
    {
	UVar v = new UVar(name);
	USound s = new USound();
	s.setSoundFormat(USoundFormat.SOUND_RAW);
	s.setRate(42);
	s.setChannels(1);
	s.setSampleSize(8);
	s.setSampleFormat(USoundSampleFormat.SAMPLE_UNSIGNED);
	s.setData(content.getBytes());
	v.setValue(s);
	s.deleteData();
	return 0;
    }

    //    int writeRI(const std::string &name, const std::string &content)
    //    {
    //	urbi::UVar v(name);
    //	urbi::UImage i = v;
    //	memcpy(i.data, content.c_str(), content.length());
    //	return 0;
    //    }
    //
    //    int writeRSnd(const std::string &name, const std::string &content)
    //    {
    //	urbi::UVar v(name);
    //	urbi::USound i = v;
    //	memcpy(i.data, content.c_str(), content.length());
    //	return 0;
    //    }

    /** Test function parameter and return value **/
    public double transmitD(double v)
    {
	return -v;
    }

    public UList transmitL(UList l)
    {
	UList r = new UList();
	for (long i = 0; i < l.size(); i++)
	    r.push_back(l.get(l.size() - i - 1));
	return r;
    }

    public UDictionary transmitM(UDictionary d)
    {
	UDictionary r = new UDictionary();
	for (Map.Entry entry : d.entrySet())
	    r.put((String) entry.getKey(), (UValue) entry.getValue());
	return r;
    }

    public String transmitS(String name)
    {
	return name.substring(1, name.length()-1);
    }

    public UBinary transmitB(UBinary b)
    {
	UBinary res = new UBinary(b);
	byte[] data = res.getData();
	for (int i = 0; i < data.length; ++i)
	    data[i] -= 1;
	data[data.length - 1] = '\n';
	res.setData(data);
	return res;
    }

    public UImage transmitI(UImage im)
    {
	UImage res = new UImage(im);
	byte[] data = res.getData();
	for (int i = 0; i < data.length; ++i)
	    data[i] -= 1;
        res.setData(data);
	return im;
    }

    public USound transmitSnd(USound im)
    {
	USound res = new USound(im);
	byte[] data = res.getData();
	for (int i = 0; i < data.length; ++i)
	    data[i] -= 1;
        res.setData(data);
	return im;
    }

    /*
      urbi::UObject* transmitO(UObject* o)
      {
      return o;
      }
    */

    public UVector transmitVector(UVector v)
    {
        Log.info("transmitVector (size = " + Double.toString(v.size()) + ")");
        for (int i = 0; i < v.size(); ++i)
            v.set(i, v.get(i) + 1);
        return v;
    }

    public UMatrix transmitMatrix(UMatrix m)
    {
        for (int i = 0; i < m.size1(); ++i)
            for (int j = 0; j < m.size2(); ++j)
                m.set(i, j, m.get(i, j) + i);
        return m;
    }

    public int sendString(String s)
    {
	send(s);
	return 0;
    }

    public int sendBuf(String b, int l)
    {
	send(b.getBytes(), l);
	return 0;
    }

    public int sendPar()
    {
        // URBI((Object.a = 123,));
        send("Object.a = 123,");
        return 0;
    }

    //    void loop_yield(long duration)
    //    {
    //	libport::utime_t end = libport::utime() + duration;
    //	while (libport::utime() < end)
    //	{
    //	    yield();
    //	    usleep(1000);
    //	}
    //    }

    public void yield_for(java.lang.Double duration)
    {
        super.yield_for((long)(duration * 1000 * 1000));
    }

    public int getDestructionCount()
    {
	Log.info("all.getDestructionCount");
	return destructionCount;
    }

    //    std::string uobjectName(UObject* n)
    //    {
    //	threadCheck();
    //	if (!n)
    //	    return std::string();
    //	else
    //	    return n->__name;
    //    }

    //    std::string allUObjectName(all* n)
    //    {
    //	return uobjectName(n);
    //    }

    public void notifyWriteA(String target, int func)
    {
	switch(func)
	    {
	    case 0:
		UNotifyChange(target, "writeAD");
		break;
	    case 1:
		UNotifyChange(target, "writeAS");
		break;
	    case 2:
		UNotifyChange(target, "writeAB");
		break;
	    case 3:
		UNotifyChange(target, "writeAV");
	    }
    }

    public void writeAD(double d) { a.setValue(d); }
    public void writeAS(String s) { a.setValue(s); }
    public void writeAB(UBinary b) { a.setValue(b); }
    public void writeAV(UValue v) { a.setValue(v); }

    public void makeCall(String obj, String func,
			 UList args)
    {
	switch((int) args.size())
	    {
	    case 0:
		call(obj, func);
		break;
	    case 1:
		call(obj, func, args.get(0));
		break;
	    case 2:
		call(obj, func, args.get(0), args.get(1));
		break;
	    case 3:
		call(obj, func, args.get(0), args.get(1), args.get(2));
		break;
	    case 4:
		call(obj, func, args.get(0), args.get(1), args.get(2));
		break;
	    default:
		throw new RuntimeException("Not implemented");
	    }
    }

    public void throwException(boolean stdexcept)
    {
	if (stdexcept)
	    throw new RuntimeException("KABOOM");
	else
	    throw new Error("KABOOM");
    }

    //    std::vector<unsigned long> socketStats()
    //    {
    //	std::vector<unsigned long> res;
    //	urbi::UClient* cl = urbi::getDefaultClient();
    //	if (!cl)
    //	    return res;
    //	res.push_back(cl->bytesSent());
    //	res.push_back(cl->bytesReceived());
    //	return res;
    //    }


    /*---------.
    | Locale.  |
    `---------*/
    public void setLocale(String cat, String loc)
    {
        // Ignore the category, it's a completely different story in
        // Java.  Besides, we might need to split the loc in two:
        // "fr_FR" => "fr", "FR".
        int pos = loc.indexOf('_');
        if (pos == -1)
            {
                Locale.setDefault(new Locale(loc));
            }
        else
            {
                String lang = loc.substring(0, pos - 1);
                String country = loc.substring(pos + 1);
                Locale.setDefault(new Locale(lang, country));
            }
    }

    public String getLocale(String cat)
    {
        return Locale.getDefault().toString();
    }



    public UVar a = new UVar(), b = new UVar(), c = new UVar(), d = new UVar();
    public UVar[] vars = new UVar[4];
    public UEvent ev = new UEvent();

    // Name of var that trigerred notifyChange.
    public UVar lastChange = new UVar();
    // Value read on said var.
    public UVar lastChangeVal = new UVar();
    // Name of var that triggered notifyAccess.
    public UVar lastAccess = new UVar();
    // Value written to said var.
    public UVar lastAccessVal = new UVar();
    // Set to 0 in ctor, 1 in init.
    public UVar initCalled = new UVar();

    // Periodic write target (0, 1 or 2 for a, b or c).
    public UVar periodicWriteTarget = new UVar();
    // Write rate (seconds).
    public UVar periodicWriteRate = new UVar();
    // Write data type.
    public UVar periodicWriteType = new UVar();

    // If an UVar with the name in removeNotify reaches a callback,
    // unnotify will be called.
    public UVar removeNotify = new UVar();
    // Number of calls to onChange
    public UVar changeCount = new UVar();
    // Cached value to ensure consistency in remote mode.
    public int count;
    public static int destructionCount = 0;
}

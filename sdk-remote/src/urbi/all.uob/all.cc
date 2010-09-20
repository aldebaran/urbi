/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <iostream>
#include <libport/compiler.hh>
#include <libport/debug.hh>
#include <libport/unistd.h>
#include <sstream>
#include <stdexcept>

#include <urbi/uclient.hh>
#undef URBI
#include <urbi/uobject.hh>

GD_CATEGORY(all);

class all: public urbi::UObject
{
public:
  typedef urbi::UObject super_type;
  pthread_t mainthread;

  inline void threadCheck()
  {
    aver(mainthread == pthread_self());
  }
  all(const std::string& name)
    : urbi::UObject(name)
  {
    count = 0;
    mainthread = pthread_self();
    if (getenv("CTOR_EXCEPTION") &&
        !strcmp(getenv("CTOR_EXCEPTION"), "true"))
    throw std::runtime_error("constructor failure");
    UBindFunction(all, init);
    UBindFunction(all, setOwned);
    UBindFunction(all, setNotifyChange);

    /** BYPASS check **/
    UBindFunction(all, setBypassNotifyChangeBinary);
    UBindFunction(all, setBypassNotifyChangeImage);
    UBindFunctions(all, markBypass, markRTP);
    UBindFunctions(all, selfWriteB, selfWriteI, selfWriteVD);

    UBindFunctions(all, setNotifyAccess, setNotifyChangeByName, setNotifyChangeByUVar, sendEvent8Args, unnotify);
    UBindFunction(all, read);
    UBindFunction(all, write);
    UBindFunction(all, readByName);
    UBindFunction(all, writeByName);
    UBindFunction(all, writeByUVar);
    UBindFunction(all, writeOwnByName);
    UBindFunction(all, urbiWriteOwnByName);
    UBindFunction(all, sendString);
    UBindFunction(all, sendBuf);
    UBindFunction(all, sendPar);
    UBindFunction(all, typeOf);
    UBindFunction(all, uobjectName);
    UBindFunction(all, allUObjectName);
    UBindVar(all,a);
    UBindVars(all, b, c, d);
    UBindVar(all, initCalled);
    initCalled = 0;
    UBindVar(all, lastChange);
    UBindVar(all, lastAccess);
    UBindVar(all, lastChangeVal);
    lastChangeVal = -1;
    UBindVar(all, lastAccessVal);
    UBindVar(all, removeNotify);
    removeNotify = "";
    // Properties.
    UBindFunction(all, readProps);
    UBindFunction(all, writeProps);

    UBindFunction(all, writeD);
    UBindFunction(all, writeS);
    UBindFunction(all, writeL);
    UBindFunction(all, writeM); // M for Map
    UBindFunction(all, writeB);
    UBindFunction(all, makeCall);
    UBindFunction(all, writeBNone);
    UBindFunction(all, writeI);
    UBindFunction(all, writeSnd);
    UBindFunction(all, writeRI);
    UBindFunction(all, writeRSnd);

    UBindFunction(all, transmitD);
    UBindFunction(all, transmitS);
    UBindFunction(all, transmitL);
    UBindFunction(all, transmitM);
    UBindFunction(all, transmitB);
    UBindFunction(all, transmitI);
    UBindFunction(all, transmitSnd);
    UBindFunction(all, transmitO);

    UBindFunction(all, loop_yield);
    UBindFunction(urbi::UContext, side_effect_free_get);
    UBindFunction(urbi::UContext, side_effect_free_set);
    UBindFunction(urbi::UContext, yield);
    UBindFunction(urbi::UContext, yield_for);
    UBindFunction(urbi::UContext, yield_until_things_changed);

    UBindFunction(all, getDestructionCount);

    UBindFunction(all, invalidRead);
    UBindFunction(all, invalidWrite);

    UBindEvent(all, ev);
    UBindFunction(all, sendEvent);
    UBindFunction(all, sendEvent2Args);
    UBindFunction(all, sendNamedEvent);

    UBindFunction(all, throwException);
    UBindFunction(all, socketStats);
    UBindVars(all, periodicWriteTarget, periodicWriteType, periodicWriteRate,
              changeCount);
    UNotifyChange(periodicWriteRate, &all::onRateChange);
    vars[0] = &a;
    vars[1] = &b;
    vars[2] = &c;
    vars[3] = &d;

    UBindFunctions(all, notifyWriteA, writeAD, writeAS, writeAB, manyWriteTest);
  }

  ~all()
  {
    ++destructionCount;
  }

  void onRateChange(urbi::UVar&)
  {
    USetUpdate((ufloat)periodicWriteRate * 1000.0);
  }
  virtual int update()
  {
    int target = periodicWriteTarget;
    int type = periodicWriteType;
    switch(type)
    {
    case urbi::DATA_STRING:
      *vars[target] = string_cast(libport::utime());
      break;
    case urbi::DATA_BINARY:
      selfWriteB(target,  string_cast(libport::utime()));
      break;
    case urbi::DATA_DOUBLE:
    default:
      *vars[target] = libport::utime();
      break;
    }
    return 0;
  }
  int setBypassNotifyChangeBinary(const std::string& name)
  {
    threadCheck();
    UNotifyChange(name, &all::onBinaryBypass);
    return 0;
  }
  int setBypassNotifyChangeImage(const std::string& name)
  {
    threadCheck();
    UNotifyChange(name, &all::onImageBypass);
    return 0;
  }
  int markBypass(int id, bool state)
  {
    threadCheck();
    return vars[id]->setBypass(state);
  }
  int markRTP(int id, bool state)
  {
    vars[id]->useRTP(state);
    return 0;
  }
  void unnotify(int id)
  {
    vars[id]->unnotify();
  }
  int onBinaryBypass(urbi::UVar& var)
  {
    threadCheck();
    const urbi::UBinary& cb = var;
    GD_FINFO_DEBUG("onbin cptr %s", cb.common.data);
    urbi::UBinary& b = const_cast<urbi::UBinary&>(cb);
    for (unsigned int i=0; i<b.common.size; ++i)
      ((char*)b.common.data)[i]++;
    return 0;
  }
  void onImageBypass(urbi::UVar& var)
  {
    threadCheck();
    const urbi::UImage& cb = var;
    GD_FINFO_DEBUG("onimg cptr %s", (void*)cb.data);
    urbi::UImage& b = const_cast<urbi::UImage&>(cb);
    for (unsigned int i=0; i<b.size; ++i)
      b.data[i]++;
  }
  std::string selfWriteB(int idx, const std::string& content)
  {
    threadCheck();
    urbi::UBinary b;
    b.type = urbi::BINARY_IMAGE;
    // Dup since we want to test no-copy op: the other end will write.
    b.common.data = strdup(content.c_str());
    b.common.size = content.length();
    GD_FINFO_DEBUG("writeB cptr %s", b.common.data);
    *vars[idx] = b;
    std::string res((char*)b.common.data, b.common.size);
    free(b.common.data);
    b.common.data = 0;
    return res;
  }
  std::string selfWriteI(int idx, const std::string& content)
  {
    threadCheck();
    urbi::UImage i;
    i.data = (unsigned char*)strdup(content.c_str());
    GD_FINFO_DEBUG("writeI cptr %s", (void*)i.data);
    i.size = content.length();
    *vars[idx] = i;
    std::string res((char*)i.data, i.size);
    free(i.data);
    return res;

  }
  int writeOwnByName(const std::string& name, int val)
  {
    threadCheck();
    urbi::UVar v(__name + "." + name);
    v = val;
    return 0;
  }

  int urbiWriteOwnByName(const std::string& name, int val)
  {
    threadCheck();
    std::stringstream ss;
    ss << __name << "." << name << " = " << val << ";";
    send(ss.str());
    return 0;
  }
  void selfWriteVD(int i, std::vector<double> v)
  {
    *vars[i] = v;
  }
  std::string typeOf(const std::string& name)
  {
    threadCheck();
    urbi::UVar v(name);
    v.syncValue();
    return v.val().format_string();
  }

  int init(int fail)
  {
    threadCheck();
    initCalled = 1;
    if (fail > 1)
      throw std::runtime_error("KABOOOM");
    return fail;
  }

  int setOwned(int id)
  {
    threadCheck();
    UOwned(*vars[id]);
    return 0;
  }

  int setNotifyChange(int id)
  {
    threadCheck();
    UNotifyChange(*vars[id], &all::onChange);
    return 0;
  }

  int setNotifyChangeByUVar(urbi::UVar& v)
  {
    threadCheck();
    UNotifyChange(v, &all::onChange);
    return 0;
  }
  int setNotifyAccess(int id)
  {
    threadCheck();
    UNotifyAccess(*vars[id], &all::onAccess);
    return 0;
  }

  int setNotifyChangeByName(const std::string& name)
  {
    threadCheck();
    UNotifyChange(name, &all::onChange);
    return 0;
  }


  int read(int id)
  {
    threadCheck();
    int v = *vars[id];
    return v;
  }
  int write(int id, int val)
  {
    threadCheck();
    *vars[id] = val;
    return val;
  }
  void invalidWrite()
  {
    threadCheck();
    urbi::UVar v;
    v = 12;
  }
  void invalidRead()
  {
    threadCheck();
    urbi::UVar v;
    int i = v;
    (void)i;
  }
  int readByName(const std::string &name)
  {
    threadCheck();
    urbi::UVar v(name);
    return v;
  }

  int writeByName(const std::string& name, int val)
  {
    threadCheck();
    urbi::UVar v(name);
    v = val;
    return val;
  }

  int writeByUVar(urbi::UVar v, urbi::UValue val)
  {
    threadCheck();
    v = val;
    return 0;
  }

  int onChange(urbi::UVar& v)
  {
    threadCheck();
    lastChange = v.get_name();
    changeCount = ++count;
    if (v.type() == urbi::DATA_DOUBLE)
    {
      int val = v;
      lastChangeVal = val;
    }
    if (removeNotify == v.get_name())
    {
      v.unnotify();
      removeNotify = "";
    }
    return 0;
  }

  int onAccess(urbi::UVar& v)
  {
    threadCheck();
    static int val = 0;
    lastAccess = v.get_name();
    val++;
    v = val;
    lastAccessVal = val;
    if ((std::string)removeNotify == v.get_name())
    {
      v.unnotify();
      removeNotify = "";
    }
    return 0;
  }

  void
  sendEvent()
  {
    threadCheck();
    ev.emit();
  }

  void
  sendEvent2Args(urbi::UValue v1, urbi::UValue v2)
  {
    ev.emit(v1, v2);
  }

  void sendEvent8Args()
  {
    ev.emit(0, "foo", 5.1, 4, 5, 6, 7, 8);
  }


  void
  sendNamedEvent(const std::string& name)
  {
    urbi::UEvent tempEv(name);
    tempEv.emit();
  }

  /// Return the value of the properties of the variable \a name.
  urbi::UList
  readProps(const std::string& name)
  {
    threadCheck();
    urbi::UVar v(name);
    urbi::UList res;

#define APPEND(Value)                                   \
    res.array.push_back(new urbi::UValue(Value))

#define APPEND_UFLOAT(Prop)                     \
    APPEND(static_cast<ufloat>(v.Prop))

    APPEND_UFLOAT(rangemin);
    APPEND_UFLOAT(rangemax);
    APPEND_UFLOAT(speedmin);
    APPEND_UFLOAT(speedmax);
    APPEND_UFLOAT(delta);
    urbi::UValue bl = v.blend;
    APPEND(bl);
    APPEND_UFLOAT(constant);
#undef APPEND_UFLOAT
#undef APPEND

    GD_FINFO_DEBUG("all.readProps: %s", res);
    return res;
  }

  int writeProps(const std::string &name, double val)
  {
    threadCheck();
    urbi::UVar v(name);
    v.rangemin = val;
    v.rangemax = val;
    v.speedmin = val;
    v.speedmax = val;
    v.delta = val;
    v.blend = val;
    v.constant = (val>0);
    return 0;
  }


  /**  Test write to UVAR.  **/

  int writeD(const std::string &name, double val)
  {
    threadCheck();
    GD_FINFO_DEBUG("writeD %s", name);
    urbi::UVar v(name);
    v = val;
    return 0;
  }

  int writeS(const std::string &name, const std::string &val)
  {
    GD_FINFO_DEBUG("writeS %s", name);
    urbi::UVar v(name);
    v = val;
    return 0;
  }

  int writeL(const std::string &name, const std::string &val)
  {
    GD_FINFO_DEBUG("writeL %s", name);
    urbi::UVar v(name);
    urbi::UList l;
    l.array.push_back(new urbi::UValue(val));
    l.array.push_back(new urbi::UValue(42));
    v = l;
    return 0;
  }

  int writeM(const std::string &name, const std::string &val)
  {
    GD_FINFO_DEBUG("writeM %s", name);
    urbi::UVar v(name);
    urbi::UDictionary d;
    d[val] = 42;
    d["foo"] = urbi::UList();
    v = d;
    return 0;
  }

  int writeB(const std::string &name, const std::string &content)
  {
    urbi::UVar v(name);
    urbi::UBinary val;
    val.type = urbi::BINARY_UNKNOWN;
    val.common.size = content.length();
    val.common.data = malloc(content.length());
    memcpy(val.common.data, content.c_str(), content.length());
    v = val;
    return 0;
  }

  int writeBNone(const std::string &name, const std::string &content)
  {
    urbi::UVar v(name);
    urbi::UBinary val;
    val.common.size = content.length();
    val.common.data = malloc(content.length());
    memcpy(val.common.data, content.c_str(), content.length());
    v = val;
    return 0;
  }

  int writeI(const std::string &name, const std::string &content)
  {
    urbi::UVar v(name);
    urbi::UImage i;
    i.imageFormat = urbi::IMAGE_JPEG;
    i.width = i.height = 42;
    i.size = content.length();
    i.data = (unsigned char*)malloc(content.length());
    memcpy(i.data, content.c_str(), content.length());
    v = i;
    free(i.data);
    return 0;
  }

  int writeSnd(const std::string &name, const std::string &content)
  {
    urbi::UVar v(name);
    urbi::USound s;
    s.soundFormat = urbi::SOUND_RAW;
    s.rate = 42;
    s.size = content.length();
    s.channels = 1;
    s.sampleSize= 8;
    s.sampleFormat = urbi::SAMPLE_UNSIGNED;
    s.data = (char*)malloc(content.length());
    memcpy(s.data, content.c_str(), content.length());
    v = s;
    free(s.data);
    return 0;
  }


  int writeRI(const std::string &name, const std::string &content)
  {
    urbi::UVar v(name);
    urbi::UImage i = v;
    memcpy(i.data, content.c_str(), content.length());
    return 0;
  }

  int writeRSnd(const std::string &name, const std::string &content)
  {
    urbi::UVar v(name);
    urbi::USound i = v;
    memcpy(i.data, content.c_str(), content.length());
    return 0;
  }


  /** Test function parameter and return value **/
  double transmitD(double v)
  {
    threadCheck();
    return -(double)v;
  }

  urbi::UList transmitL(urbi::UList l)
  {
    urbi::UList r;
    for (unsigned int i=0; i<l.array.size(); i++)
      r.array.push_back(new urbi::UValue(*l.array[l.array.size()-i-1]));
    return r;
  }

  urbi::UDictionary transmitM(urbi::UDictionary d)
  {
    urbi::UDictionary r;
    foreach (const urbi::UDictionary::value_type& t, d)
      r[t.first] = t.second;
    return r;
  }

  std::string transmitS(const std::string &name)
  {
    return name.substr(1, name.length()-2);
  }

  urbi::UBinary transmitB(urbi::UBinary b)
  {
    urbi::UBinary res(b);
    unsigned char* data = static_cast<unsigned char*>(res.common.data);
    for (size_t i = 0; i < res.common.size; ++i)
      data[i] -= 1;
    data[res.common.size - 1] = '\n';
    return res;
  }

  urbi::UImage transmitI(urbi::UImage im)
  {
    for (unsigned int i=0; i<im.size; i++)
      im.data[i] -= 1;
    return im;
  }

  urbi::USound transmitSnd(urbi::USound im)
  {
    for (unsigned int i=0; i<im.size; i++)
      im.data[i] -= 1;
    return im;
  }

  urbi::UObject* transmitO(UObject* o)
  {
    return o;
  }

  int sendString(const std::string& s)
  {
    threadCheck();
    send(s.c_str());
    return 0;
  }
  int sendBuf(const std::string& b, int l)
  {
    send(const_cast<void*>(static_cast<const void*>(b.c_str())), l);
    return 0;
  }
  int sendPar()
  {
    URBI((Object.a = 123,));
    return 0;
  }

  void loop_yield(long duration)
  {
    libport::utime_t end = libport::utime() + duration;
    while (libport::utime() < end)
    {
      yield();
      usleep(1000);
    }
  }

  int getDestructionCount()
  {
    return destructionCount;
  }

  std::string uobjectName(UObject* n)
  {
    threadCheck();
    if (!n)
      return std::string();
    else
      return n->__name;
  }

  std::string allUObjectName(all* n)
  {
    return uobjectName(n);
  }

  void notifyWriteA(const std::string& target, int func)
  {
    switch(func)
    {
    case 0:
      UNotifyChange(target, &all::writeAD);
      break;
    case 1:
      UNotifyChange(target, &all::writeAS);
      break;
    case 2:
      UNotifyChange(target, &all::writeAB);
      break;
    case 3:
      UNotifyChange(target, &all::writeAV);
    }
  }
  void writeAD(double d) { a = d; }
  void writeAS(const std::string& s) {a = s;}
  void writeAB(urbi::UBinary b) { a=b;}
  void writeAV(urbi::UValue v) { a=v;}

  void makeCall(const std::string& obj, const std::string& func,
                urbi::UList args)
  {
    threadCheck();
     switch(args.size())
     {
     case 0:
       call(obj, func);
       break;
     case 1:
       call(obj, func, args[0]);
       break;
     case 2:
       call(obj, func, args[0], args[1]);
       break;
     case 3:
       call(obj, func, args[0], args[1], args[2]);
       break;
     case 4:
       call(obj, func, args[0], args[1], args[2]);
       break;
     default:
       throw std::runtime_error("Not implemented");
     }
  }

  void throwException(bool stdexcept)
  {
    if (stdexcept)
      throw std::runtime_error("KABOOM");
    else
      throw "KABOOM";
  }

  std::vector<unsigned long> socketStats()
  {
    std::vector<unsigned long> res;
    urbi::UClient* cl = urbi::getDefaultClient();
    if (!cl)
      return res;
   res.push_back(cl->bytesSent());
   res.push_back(cl->bytesReceived());
   return res;
  }

  // Test templates UVar.as, UVar.==, UVar.fill
  void manyWriteTest(int i)
  {
    #define CHECK                                     \
    if (d.size() !=2 || d[0] != 1 || d[1] != 2)       \
      throw std::runtime_error("vector is not what we expect")
    std::vector<double> d;
    d.push_back(1);
    d.push_back(2);
    urbi::UVar& v =*vars[i];
    v = d;
    d = v.as(&d);
    CHECK;
    d = v.as<std::vector<double> >();
    CHECK;
    v.fill(d);
    if (!(v == d))
      throw std::runtime_error("v != d");
    CHECK;
    boost::unordered_map<std::string, UObject*> vu;
    vu["coin"] = this;
    a = vu;
    a.fill(vu);
    if (!libport::mhas(vu, "coin") || vu["coin"] != this)
      throw std::runtime_error("hash is not what we expect");
  }

  urbi::UVar a, b, c, d;
  urbi::UVar* vars[4];
  urbi::UEvent ev;

  //name of var that trigerred notifyChange
  urbi::UVar lastChange;
  //value read on said var
  urbi::UVar lastChangeVal;
  //name of var that triggered notifyAccess
  urbi::UVar lastAccess;
  //value written to said var
  urbi::UVar lastAccessVal;
  //Set to 0 in ctor, 1 in init
  urbi::UVar initCalled;

  // Periodic write target (0, 1 or 2 for a, b or c)
  urbi::UVar periodicWriteTarget;
  // Write rate (seconds)
  urbi::UVar periodicWriteRate;
  // Write data type
  urbi::UVar periodicWriteType;

  // If an UVar with the name in removeNotify reaches a callback,
  // unnotify will be called.
  urbi::UVar removeNotify;
  // Number of calls to onChange
  urbi::UVar changeCount;
  // Cached value to ensure consistency in remote mode.
  int count;
  static int destructionCount;
};

int all::destructionCount = 0;

::urbi::URBIStarter<all>
    starter1(urbi::isPluginMode() ? "all"  : "remall"),
    starter2(urbi::isPluginMode() ? "all2" : "remall2");

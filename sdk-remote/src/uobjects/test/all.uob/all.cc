/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <iostream>
#include <libport/umatrix.hh>
#include <libport/compiler.hh>
#include <libport/containers.hh>
#include <libport/debug.hh>
#include <libport/locale.hh>
#include <libport/ufloat.hh>
#include <libport/unistd.h>
#include <libport/utime.hh>
#include <sstream>
#include <stdexcept>

#include <urbi/uclient.hh>
#include <urbi/uconversion.hh>
#undef URBI
#include <urbi/uobject.hh>
#include <urbi/customuvar.hh>
#include <urbi/input-port.hh>

GD_CATEGORY(Test.All);

/// Same as strdup, but without the \0 limitation.
static void*
memdup(const void *data, size_t size)
{
  return memcpy(malloc(size), data, size);
}

static void*
memdup(const std::string& s)
{
  return memdup(s.c_str(), s.size());
}

struct Point
{
  Point()
    : x(0)
    , y(0)
  {}
  urbi::ufloat x,y;
};

struct Rect
{
  Point a;
  Point b;
};

struct PointOfInterest
{
  std::string sectorName;
  std::vector<Rect> subSectors;
  boost::unordered_map<std::string, Rect> byName;
};

URBI_REGISTER_STRUCT(Point, x, y);
URBI_REGISTER_STRUCT(Rect, a, b);
URBI_REGISTER_STRUCT(PointOfInterest, sectorName, subSectors, byName);


class all: public urbi::UObject
{
public:
  typedef urbi::UObject super_type;
  pthread_t mainthread;

  inline void threadCheck() const
  {
    aver(mainthread == pthread_self());
  }

  all()
  {
    GD_INFO_TRACE("all default ctor");
    count = 0;
    mainthread = pthread_self();
    setup();
    init(1);
  }

  all(const std::string& name)
    : urbi::UObject(name)
  {
    GD_INFO_TRACE("all string ctor");
    count = 0;
    mainthread = pthread_self();
    if (getenv("CTOR_EXCEPTION") &&
        !strcmp(getenv("CTOR_EXCEPTION"), "true"))
      throw std::runtime_error("constructor failure");
    setup();
  }

  void setup()
  {
    UBindFunction(all, init);
    UBindFunction(all, setOwned);
    UBindFunctions(all, multiRead, multiWrite);

    /** BYPASS check **/
    UBindFunctions(all,
                   setBypassNotifyChangeBinary, setBypassNotifyChangeImage);
    UBindFunctions(all, markBypass, markRTP);
    UBindFunctions(all, selfWriteB, selfWriteI, selfWriteVD);

    UBindFunctions
      (all,
       setNotifyAccess,
       setNotifyChange, setNotifyChangeByName, setNotifyChangeByUVar,
       sendEvent8Args,
       unnotify,
       setThreadedNotifyChange, setThreadedNotifyChangeByUVar);
    UBindFunctions
      (all,
       read, write,
       readByName, readValByName, writeByName, writeByUVar,
       writeOwnByName, urbiWriteOwnByName, sendString,
       sendBuf, sendPar, typeOf, uobjectName, allUObjectName);
    UBindVar(all,a);
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

    UBindFunctions
      (all, writeD, writeS, writeL, writeM,
       writeB, makeCall, writeBNone, writeI, writeSnd,
       writeRI, writeRSnd);

    UBindFunctions
      (all,
       transmitD, transmitS, transmitL, transmitM, transmitB,
       transmitI, transmitSnd, transmitO,
       transmitVector, transmitMatrix);
    UBindFunction(all, emitO);
    UBindFunctions
      (all,
       loop_yield, yield_for);
    UBindFunctions
      (urbi::UContext,
       side_effect_free_get, side_effect_free_set,
       yield);

    UBindFunction(all, getDestructionCount);

    UBindFunctions(all, invalidRead, invalidWrite);

    UBindEvent(all, ev);
    UBindFunctions
      (all,
       sendEvent, sendEvent2Args, sendNamedEvent);

    UBindFunction(all, throwException);
    UBindFunction(all, socketStats);
    UBindFunction(all, instanciate);
    UBindFunctions(all,
                   area, translate, makeRect, multiTranslate,
                   transmitPointOfInterest, writePointOfInterest,
                   readPointOfInterest);
    UBindFunctionRename(all, area, "rectArea");
    UBindFunctions(all, pack, unpack);
    UBindVars(all, periodicWriteTarget, periodicWriteType, periodicWriteRate,
              periodicWriteCount, changeCount);

    UBindFunctions(all, setLocale, getLocale);

    // Image conversion.
    UBindFunctions(all, convert, imageDiff);

    UBindCacheVar(all, writeLastChangeVal, bool);
    writeLastChangeVal = true;
    periodicWriteCount = 1;
    UNotifyChange(periodicWriteRate, &all::onRateChange);
    vars[0] = &a;
    vars[1] = &b;
    vars[2] = &c;
    vars[3] = &d;
    vars[4] = &unbound;
    for(int i = 0; i < 4; ++i)
      ports[i] =
        new urbi::InputPort(this, libport::format("i%s", char('a' + i)));

    // 4 is unbound
    ports[4] = new urbi::InputPort();
    UBindFunctions(all, notifyWriteA, writeAD, writeAS, writeAB, writeAV,
                   manyWriteTest);
    UAt(all, boundev);
  }

  ~all()
  {
    ++destructionCount;
  }

  urbi::UImage
  convert(const urbi::UImage source, const std::string& id)
  {
    urbi::UImage res;
    res.make();
    res.imageFormat = urbi::parse_image_format(id);
    urbi::convert(source, res);
    return res;
  }

  size_t
  bufDiff(const unsigned char* b1, const unsigned char* b2,
          size_t size,
          const std::string& ctx)
  {
    size_t res = 0;

    // Number of errors.
    size_t num = 0;
    for (size_t i = 0; i < size; ++i)
    {
      if (size_t d = abs(b2[i] - b1[i]))
      {
        ++num;
        enum { max_errors = 5 };
        if (num <= max_errors)
          GD_FERROR("%s: at %d, %d != %d (d = %d)%s",
                    ctx, i, int(b1[i]), int(b2[i]), d,
                    num == max_errors
                    ? " (too many errors, won't report others)"
                    : "");
        if (res < d)
          res = d;
      }
    }
    if (res)
      GD_FERROR("%s: %d diffs, largest: %d", ctx, num, res);
    return res;
  }

  size_t
  imageDiff(const std::string& img1,
            const std::string& img2,
            const std::string& conversion)
  {
    size_t res = 0;
#define COMPARE(Feature)                                        \
    if (img1.Feature() != img2.Feature())                       \
    {                                                           \
      ++res;                                                    \
      GD_FWARN("imageDiff: %s: "                                \
               "incompatible image " #Feature ": %d, %d",       \
               conversion, img1.Feature(), img2.Feature());     \
    }
    COMPARE(size);
#undef COMPARE

    // No need to go further if we already have differences.
    if (!res)
    {
      // The have the same size.
      res += bufDiff(reinterpret_cast<const unsigned char*>(img1.c_str()),
                     reinterpret_cast<const unsigned char*>(img2.c_str()),
                     img1.size(),
                     "imageDiff: " + conversion);
    }
    return res;
  }

  void multiWrite(int idx, int count, urbi::ufloat val)
  {
    for (int i = 0; i < count; ++i)
      *vars[idx] = val + idx;
  }

  void multiRead(int idx, int count)
  {
    urbi::ufloat sum = 0;
    for (int i = 0; i < count; ++i)
      sum += (urbi::ufloat)*vars[idx];
  }

  void boundev(urbi::ufloat v)
  {
    lastChange = "boundev";
    lastChangeVal = v;
  }

  void onRateChange(urbi::UVar&)
  {
    USetUpdate((urbi::ufloat)periodicWriteRate * 1000.0);
  }

  UObject* instanciate() const
  {
    return new all;
  }

  virtual int update()
  {
    int target = periodicWriteTarget;
    int type = periodicWriteType;
    GD_FINFO_DEBUG("update: type = %s, target = %s", type, target);
    for (int i = 0; i < (int)periodicWriteCount; ++i)
    switch (type)
    {
    case urbi::DATA_STRING:
      *vars[target] = string_cast(libport::utime());
      break;
    case urbi::DATA_BINARY:
      selfWriteB(target, string_cast(libport::utime()));
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
    if (id < 5)
      vars[id]->unnotify();
    else
      ports[id - 5]->unnotify();
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
    b.type = urbi::BINARY_UNKNOWN;
    // Dup since we want to test no-copy op: the other end will write.
    b.common.data = memdup(content);
    b.common.size = content.size();
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
    i.init();
    i.data = (unsigned char*)memdup(content);
    i.size = content.length();
    GD_FINFO_DEBUG("writeI cptr %s", (void*)i.data);
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

  void selfWriteVD(int i, std::vector<urbi::ufloat> v)
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


  /*---------.
  | Notify.  |
  `---------*/
// InputPort is not a UVar (private inheritance), so we are not
// allowed to bounce from setNotifyChange to setNotifyChangeByUVar for
// instance, since that requires to see *ports[0] as an UVar, which we
// are not entitled to see.
//
// UObject is a friend, so UNotifyChange is allowed to do that.
#define UVAR_DISPATH(Function, ...)             \
  do {                                          \
    threadCheck();                              \
    if (id < 5)                                 \
      Function(*vars[id], __VA_ARGS__);         \
    else                                        \
      Function(*ports[id - 5], __VA_ARGS__);    \
    return 0;                                   \
  } while (false)

  int setNotifyChangeByUVar(urbi::UVar& v)
  {
    threadCheck();
    UNotifyChange(v, &all::onChange);
    return 0;
  }

  int setNotifyChange(int id)
  {
    UVAR_DISPATH(UNotifyChange, &all::onChange);
  }

  int setThreadedNotifyChangeByUVar(urbi::UVar& v)
  {
    UNotifyThreadedChange(v, &all::onThreadedChange, urbi::LOCK_FUNCTION);
    return 0;
  }

  int setThreadedNotifyChange(int id)
  {
    UVAR_DISPATH(UNotifyThreadedChange,
                 &all::onThreadedChange, urbi::LOCK_FUNCTION);
  }

  int setNotifyAccess(int id)
  {
    UVAR_DISPATH(UNotifyAccess, &all::onAccess);
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
    LIBPORT_USE(i);
  }

  int readByName(const std::string& name)
  {
    threadCheck();
    urbi::UVar v(name);
    return v;
  }

  urbi::UValue readValByName(const std::string& name)
  {
    urbi::UVar v(name);
    return v.val();
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

  int onThreadedChange(urbi::UValue v)
  {
    lastChange = "<unknown>";
    changeCount = ++count;
    lastChangeVal = v;
    return 0;
  }

  int onChange(urbi::UVar& v)
  {
    GD_INFO_DUMP("entering onChange");
    lastChange = v.get_name();
    changeCount = ++count;
    if (writeLastChangeVal.data())
    {
      if (v.type() == urbi::DATA_DOUBLE)
      {
        GD_FINFO_DUMP("onChange double %s", v.get_name());
        int val = v;
        lastChangeVal = val;
      }
      else if (v.type() == urbi::DATA_BINARY)
      {
        GD_FINFO_DUMP("onChange binary %s", v.get_name());
        urbi::UBinary b = v;
        lastChangeVal = b;
      }
      else
        GD_FINFO_DUMP("onChange unknown %s", v.get_name());
    }
    if (removeNotify == v.get_name())
    {
      v.unnotify();
      removeNotify = "";
    }
    GD_INFO_DUMP("exiting onChange");
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
    ev.emit(0, "foo", 5.1, 4, 5, 6, 7);
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
    APPEND(static_cast<urbi::ufloat>(v.Prop))

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

  int writeProps(const std::string& name, urbi::ufloat val)
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

  int writeD(const std::string& name, urbi::ufloat val)
  {
    threadCheck();
    GD_FINFO_DEBUG("writeD %s", name);
    urbi::UVar v(name);
    v = val;
    return 0;
  }

  int writeS(const std::string& name, const std::string& val)
  {
    GD_FINFO_DEBUG("writeS %s", name);
    urbi::UVar v(name);
    v = val;
    return 0;
  }

  int writeL(const std::string& name, const std::string& val)
  {
    GD_FINFO_DEBUG("writeL %s", name);
    urbi::UVar v(name);
    urbi::UList l;
    l.array.push_back(new urbi::UValue(val));
    l.array.push_back(new urbi::UValue(42));
    v = l;
    return 0;
  }

  int writeM(const std::string& name, const std::string& val)
  {
    GD_FINFO_DEBUG("writeM %s", name);
    urbi::UVar v(name);
    urbi::UDictionary d;
    d[val] = 42;
    d["foo"] = urbi::UList();
    v = d;
    return 0;
  }

  int writeB(const std::string& name, const std::string& content)
  {
    urbi::UVar v(name);
    urbi::UBinary val;
    val.type = urbi::BINARY_UNKNOWN;
    val.common.size = content.length();
    val.common.data = memdup(content);
    v = val;
    return 0;
  }

  int writeBNone(const std::string& name, const std::string& content)
  {
    urbi::UVar v(name);
    urbi::UBinary val;
    val.common.size = content.length();
    val.common.data = memdup(content);
    v = val;
    return 0;
  }

  int writeI(const std::string& name, const std::string& content)
  {
    urbi::UVar v(name);
    urbi::UImage i;
    i.init();
    i.imageFormat = urbi::IMAGE_JPEG;
    i.width = i.height = 42;
    i.size = content.length();
    i.data = (unsigned char*)memdup(content);
    v = i;
    free(i.data);
    return 0;
  }

  int writeSnd(const std::string& name, const std::string& content)
  {
    urbi::UVar v(name);
    urbi::USound s;
    s.init();
    s.soundFormat = urbi::SOUND_RAW;
    s.rate = 42;
    s.size = content.length();
    s.channels = 1;
    s.sampleSize= 8;
    s.sampleFormat = urbi::SAMPLE_UNSIGNED;
    s.data = (char*)memdup(content);
    v = s;
    free(s.data);
    return 0;
  }


  int writeRI(const std::string& name, const std::string& content)
  {
    urbi::UVar v(name);
    urbi::UImage i = v;
    memcpy(i.data, content.c_str(), content.length());
    return 0;
  }

  int writeRSnd(const std::string& name, const std::string& content)
  {
    urbi::UVar v(name);
    urbi::USound i = v;
    memcpy(i.data, content.c_str(), content.length());
    return 0;
  }


  /** Test function parameter and return value **/
  urbi::ufloat transmitD(urbi::ufloat v) const
  {
    threadCheck();
    return -v;
  }

  urbi::UList transmitL(urbi::UList l) const
  {
    urbi::UList r;
    for (unsigned int i=0; i<l.array.size(); ++i)
      r.array.push_back(new urbi::UValue(*l.array[l.array.size()-i-1]));
    return r;
  }

  urbi::UDictionary transmitM(urbi::UDictionary d) const
  {
    urbi::UDictionary r;
    foreach (const urbi::UDictionary::value_type& t, d)
      r[t.first] = t.second;
    return r;
  }

  std::string transmitS(const std::string& name) const
  {
    return name.substr(1, name.length()-2);
  }

  urbi::UBinary transmitB(urbi::UBinary b) const
  {
    urbi::UBinary res(b);
    unsigned char* data = static_cast<unsigned char*>(res.common.data);
    for (size_t i = 0; i < res.common.size; ++i)
      data[i] -= 1;
    data[res.common.size - 1] = '\n';
    return res;
  }

  urbi::UImage transmitI(urbi::UImage im) const
  {
    for (unsigned int i = 0; i < im.size; ++i)
      im.data[i] -= 1;
    return im;
  }

  urbi::USound transmitSnd(urbi::USound im) const
  {
    for (unsigned int i = 0; i < im.size; ++i)
      im.data[i] -= 1;
    return im;
  }

  urbi::UObject* transmitO(UObject* o) const
  {
    return o;
  }

  void emitO(UObject* o)
  {
    ev.emit(o);
  }

  libport::vector_type
  transmitVector(libport::vector_type v)
  {
    for (unsigned i = 0; i < v.size(); ++i)
      v(i) += 1;
    return v;
  }

  libport::matrix_type
  transmitMatrix(libport::matrix_type m)
  {
    for (unsigned i = 0; i < m.size1(); ++i)
      for (unsigned j = 0; j < m.size2(); ++j)
        m(i, j) += i;
    return m;
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

  void loop_yield(urbi::ufloat duration)
  {
    using namespace libport;
    for (utime_t end = utime() + seconds_to_utime(duration);
         utime() < end;
         usleep(1000))
      yield();
  }

  // Override from UContext.
  // Not polymorphic, as we change the signature (ufloat).
  void yield_for(urbi::ufloat duration)
  {
    urbi::UContext::yield_for(libport::seconds_to_utime(duration));
  }

  int getDestructionCount() const
  {
    return destructionCount;
  }

  std::string uobjectName(UObject* n)
  {
    threadCheck();
    return n ? n->__name : std::string();
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

  void writeAD(urbi::ufloat d) { a = d; }

  void writeAS(const std::string& s) {a = s;}

  void writeAB(urbi::UBinary b) { a=b;}

  void writeAV(urbi::UValue v) { a=v;}

  void makeCall(const std::string& obj, const std::string& func,
                urbi::UList args)
  {
    threadCheck();
    switch (args.size())
    {
#define CASE(Num, ...)                                  \
      case Num: call(obj, func, ## __VA_ARGS__); break;
      CASE(0);
      CASE(1, args[0]);
      CASE(2, args[0], args[1]);
      CASE(3, args[0], args[1], args[2]);
      CASE(4, args[0], args[1], args[2], args[3]);
#undef CASE
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
    if (urbi::UClient* cl = urbi::getDefaultClient())
      res << cl->bytesSent()
          << cl->bytesReceived();
   return res;
  }

  // Test templates UVar.as, UVar.==, UVar.fill
  void manyWriteTest(int i)
  {
#define CHECK                                                   \
    if (d.size() !=2 || d[0] != 1 || d[1] != 2)                 \
      throw std::runtime_error("vector is not what we expect")
    std::vector<urbi::ufloat> d;
    d.push_back(1);
    d.push_back(2);
    urbi::UVar& v =*vars[i];
    v = d;
    d = v.as(&d);
    CHECK;
    d = v.as<std::vector<urbi::ufloat> >();
    CHECK;
    v.fill(d);
    if (!(v == d))
      throw std::runtime_error("v != d");
    CHECK;
    boost::unordered_map<std::string, UObject*> vu;
    vu["coin"] = this;
    a = vu;
    a.fill(vu);
    if (!libport::has(vu, "coin") || vu["coin"] != this)
      throw std::runtime_error("hash is not what we expect");
  }

  urbi::ufloat area(Rect r)
  {
    return libport::round((r.a.x-r.b.x) * (r.a.y  - r.b.y));
  }

  PointOfInterest transmitPointOfInterest(PointOfInterest p)
  {
    return p;
  }

  void writePointOfInterest(urbi::UVar& target, PointOfInterest p)
  {
    target = p;
  }

  PointOfInterest readPointOfInterest(urbi::UVar& source)
  {
    source.syncValue();
    return source.as((PointOfInterest*)0);
  }

  Rect translate(Rect r, Point v)
  {
    r.a.x += v.x;
    r.a.y += v.y;
    r.b.x += v.x;
    r.b.y += v.y;
    return r;
  }

  Rect makeRect()
  {
    return Rect();
  }

  std::vector<Rect> multiTranslate(std::vector<Rect> src, Point v)
  {
    foreach(Rect& r, src)
      r = translate(r, v);
    return src;
  }

  std::vector<urbi::ufloat> unpack(UPackedData<urbi::ufloat> d)
  {
    return d;
  }

  UPackedData<urbi::ufloat> pack(std::vector<urbi::ufloat> d)
  {
    return d;
  }

  /*---------.
  | Locale.  |
  `---------*/
  void
  setLocale(const std::string& cat, const std::string& loc)
  {
    libport::setlocale(cat, loc);
  }

  std::string
  getLocale(const std::string& cat)
  {
    return libport::setlocale(cat, 0);
  }



  urbi::UVar a, b, c, d, unbound;
  urbi::UVar* vars[10];
  urbi::InputPort* ports[5];
  urbi::UEvent ev;

  //name of var that trigerred notifyChange
  urbi::UVar lastChange;
  //value read on said var
  urbi::UVar lastChangeVal;
  // Only update lastChangeVal if true (default)
  urbi::CustomUVar<bool> writeLastChangeVal;
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
  // Number of writes per call
  urbi::UVar periodicWriteCount;

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
URBI_CHECK_SDK_VERSION_BARE("all");

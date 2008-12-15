#include <iostream>
#include <sstream>

#include <urbi/uobject.hh>

class all: public urbi::UObject
{
public:
  all(const std::string& name)
    : urbi::UObject(name)
  {
    UBindFunction(all, init);
    UBindFunction(all, setOwned);
    UBindFunction(all, setNotifyChange);
    UBindFunction(all, setNotifyAccess);
    UBindFunction(all, setNotifyChangeByName);

    UBindFunction(all, read);
    UBindFunction(all, write);
    UBindFunction(all, readByName);
    UBindFunction(all, writeByName);
    UBindFunction(all, writeOwnByName);
    UBindFunction(all, urbiWriteOwnByName);
    UBindFunction(all, sendString);
    UBindFunction(all, sendBuf);
    UBindFunction(all, sendPar);
    UBindFunction(all, typeOf);

    UBindVar(all,a);
    UBindVar(all,b);
    UBindVar(all,c);
    UBindVar(all, initCalled);
    initCalled = 0;
    UBindVar(all, lastChange);
    UBindVar(all, lastAccess);
    UBindVar(all, lastChangeVal);
    lastChangeVal = -1;
    UBindVar(all, lastAccessVal);


    UBindFunction(all, readProps);
    UBindFunction(all, writeProps);
    UBindFunction(all, writeD);
    UBindFunction(all, writeS);
    UBindFunction(all, writeL);
    UBindFunction(all, writeB);
    UBindFunction(all, writeBNone);
    UBindFunction(all, writeI);
    UBindFunction(all, writeSnd);
    UBindFunction(all, writeRI);
    UBindFunction(all, writeRSnd);

    UBindFunction(all, transmitD);
    UBindFunction(all, transmitS);
    UBindFunction(all, transmitL);
    UBindFunction(all, transmitB);
    UBindFunction(all, transmitI);
    UBindFunction(all, transmitSnd);

    UBindFunction(all, yield);
    UBindFunction(all, loop_yield);
    UBindFunction(all, yield_for);
    UBindFunction(all, yield_until_things_changed);
    UBindFunction(all, side_effect_free_set);
    UBindFunction(all, side_effect_free_get);

    vars[0] = &a;
    vars[1] = &b;
    vars[2] = &c;
  }

  int writeOwnByName(const std::string& name, int val)
  {
    urbi::UVar v(__name + "." + name);
    v = val;
    return 0;
  }

  int urbiWriteOwnByName(const std::string& name, int val)
  {
    std::stringstream ss;
    ss << __name << "." << name << " = " << val << ";";
    send(ss.str());
    return 0;
  }

  int typeOf(const std::string& name)
  {
    urbi::UVar v(name);
    v.syncValue();
    return v.type();
  }

  int init(bool fail)
  {
    initCalled = 1;
    return fail ? 1 : 0;
  }

  int setOwned(int id)
  {
    UOwned(*vars[id]);
    return 0;
  }

  int setNotifyChange(int id)
  {
    UNotifyChange(*vars[id], &all::onChange);
    return 0;
  }

  int setNotifyAccess(int id)
  {
    UNotifyAccess(*vars[id], &all::onAccess);
    return 0;
  }

  int setNotifyChangeByName(const std::string& name)
  {
    UNotifyChange(name, &all::onChange);
    return 0;
  }


  int read(int id)
  {
    int v = *vars[id];
    return v;
  }
  int write(int id, int val)
  {
    *vars[id] = val;
    return val;
  }

  int readByName(const std::string &name)
  {
    urbi::UVar v(name);
    return v;
  }

  int writeByName(const std::string& name, int val)
  {
    urbi::UVar v(name);
    v = val;
    return val;
  }

  int onChange(urbi::UVar& v)
  {
    int val = v;
    lastChange = v.get_name();
    lastChangeVal = val;
    return 0;
  }

  int onAccess(urbi::UVar& v)
  {
    static int val = 0;
    lastAccess = v.get_name();
    val++;
    v = val;
    lastAccessVal = val;
    return 0;
  }


  urbi::UList readProps(const std::string &name)
  {
    urbi::UVar v(name);
    urbi::UList l;
    l.array.push_back(new urbi::UValue((double)v.rangemin));
    l.array.push_back(new urbi::UValue((double)v.rangemax));
    l.array.push_back(new urbi::UValue((double)v.speedmin));
    l.array.push_back(new urbi::UValue((double)v.speedmax));
    l.array.push_back(new urbi::UValue((double)v.delta));
    urbi::UValue bl = v.blend;
    l.array.push_back(new urbi::UValue(bl));
    return l;
  }

  int writeProps(const std::string &name, double val)
  {
    urbi::UVar v(name);
    v.rangemin = val;
    v.rangemax = val;
    v.speedmin = val;
    v.speedmax = val;
    v.delta = val;
    v.blend = val;
    return 0;
  }


  /**  Test write to UVAR.  **/

  int writeD(const std::string &name, double val)
  {
    std::cerr << "writeD " << name << std::endl;
    urbi::UVar v(name);
    v = val;
    return 0;
  }

  int writeS(const std::string &name, const std::string &val)
  {
    std::cerr << "writeS " << name << std::endl;
    urbi::UVar v(name);
    v = val;
    return 0;
  }

  int writeL(const std::string &name, const std::string &val)
  {
    std::cerr << "writeL " << name << std::endl;
    urbi::UVar v(name);
    urbi::UList l;
    l.array.push_back(new urbi::UValue(val));
    l.array.push_back(new urbi::UValue(42));
    v = l;
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
    return -(double)v;
  }

  urbi::UList transmitL(urbi::UList l)
  {
    urbi::UList r;
    for (unsigned int i=0; i<l.array.size(); i++)
      r.array.push_back(new urbi::UValue(*l.array[l.array.size()-i-1]));
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

  int sendString(const std::string& s)
  {
    urbi::send(s.c_str());
    return 0;
  }
  int sendBuf(const std::string& b, int l)
  {
    urbi::send(const_cast<void*>(static_cast<const void*>(b.c_str())), l);
    return 0;
  }
  int sendPar()
  {
    URBI((Object.a = 123,));
    return 0;
  }

  void yield()
  {
    urbi::yield();
  }
  void loop_yield(long duration)
  {
    libport::utime_t end = libport::utime() + duration;
    while (libport::utime() < end)
    {
      urbi::yield();
      usleep(1000);
    }
  }
  void yield_for(long duration)
  {
    urbi::yield_until(libport::utime() + duration);
  }
  void yield_until_things_changed()
  {
    urbi::yield_until_things_changed();
  }
  void side_effect_free_set(bool sef)
  {
    urbi::side_effect_free_set(sef);
  }
  bool side_effect_free_get()
  {
    return urbi::side_effect_free_get();
  }

  urbi::UVar a,b,c;
  urbi::UVar* vars[3];

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
};


::urbi::URBIStarter<all>
 starter1(urbi::isPluginMode()?"all":"remall");
::urbi::URBIStarter<all>
 starter2(urbi::isPluginMode()?"all2":"remall2");

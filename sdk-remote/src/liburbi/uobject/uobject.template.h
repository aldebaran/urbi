/*! \file uobject.h
 *******************************************************************************

 File: uobject.h\n
 Definition of the UObject class and necessary related classes.

 This file is part of UObject Component Architecture\n
 (c) 2006 Gostai S.A.S.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#ifndef UOBJECT_H_DEFINED
#define UOBJECT_H_DEFINED

#include <string>
#include <list>

// Hash maps, depending on the environment
#ifdef _MSC_VER
#include <hash_map>
using std::hash_map;
#else
#include <hash_map.h>
#endif

// Short cuts
using std::string;
using std::list;
using std::vector;

// Floating point definition (emulated or real)
#ifdef HAVE_UFLOAT_H
#include "ufloat.h"
#else
typedef double ufloat;
#endif

/** Singleton smart pointer that creates the object on demmand
 * */
template<class T> class SingletonPtr {
  public:
    operator T* () {return check();}
    operator T& () {return *check();}

    T* operator ->() {return check();}
    T * check() {static T * ptr=0; if(ptr) return ptr; else return (ptr=new T());}
  private:
};

#define STATIC_INSTANCE(cl, name) \
  SingletonPtr<cl##name> name

#define EXTERN_STATIC_INSTANCE(cl, name)\
  class cl##name: public cl{}; \
extern SingletonPtr<cl##name> name



// A quick hack to be able to use hash_map with string easily
#ifndef _MSC_VER

  #if (__GNUC__ == 2)
  __STL_BEGIN_NAMESPACE
  #else
  namespace __gnu_cxx {
  #endif
	  
template<> struct hash< std::string > {  
   size_t operator()( const std::string& x ) const
     { return hash< const char* >()( x.c_str() );}
};

  #if (__GNUC__ == 2)
  __STL_END_NAMESPACE
  #else
  }
  #endif

#else //_MSC_VER

  _STD_BEGIN
  template<> class hash_compare<std::string> {
  public:
    enum
    {	// parameters for hash table
	bucket_size = 4,	// 0 < bucket_size
	min_buckets = 8
    };	// min_buckets = 2 ^^ N, 0 < N

    size_t operator()( const std::string& x ) const
    { return hash_compare< const char* >()( x.c_str() );}

    bool operator()(const string& _Keyval1, const string& _Keyval2) const
    {	// test if _Keyval1 ordered before _Keyval2
	return (_Keyval1< _Keyval2);
    }
};
_STD_END
#endif


// Simply use: UStart(myUObjectType) and the rest will be taken care of.
#define UStart(x) urbi::URBIStarter<x> x ## ____URBI_object(string(#x),objectlist)
// Simply use: UStartHub(myUObjectHubType) and the rest will be taken care of.
#define UStartHub(x) urbi::URBIStarterHub<x> x ## ____URBI_object(string(#x),objecthublist)


// Variable binding
#define UBindVar(obj,x) x.init(__name,#x)

// Used for sensor+actuator variables
#define USensor(x) x.setOwned()

// Function Binding
#define UBindFunction(obj,x)  createUCallback((string)"function", this,(&obj::x),__name+"."+string(#x),functionmap)

// Event Binding
#define UBindEvent(obj,x)     createUCallback("event",    this,(&obj::x),__name+"."+string(#x),eventmap)
#define UBindEventEnd(obj,x,fun) createUCallback("eventend", this,(&obj::x),(&obj::fun),__name+"."+string(#x),eventendmap)

// Macro to register to a Hub
#define URegister(hub) { objecthub = urbi::getUObjectHub((string)#hub); \
  if (objecthub) objecthub->addMember(dynamic_cast<UObject*>(this)); \
  else echo("Error: hub name '%s' is unknown\n",#hub); }

// defines a variable and it's associated accessors
#define PRIVATE(vartype,varname) private: vartype varname;public: vartype get_ ## varname \
  () { return varname; } void set_ ## varname (vartype& ____value) { varname = ____value; }  private:


//macro to send urbi commands
#ifndef URBI
  #define URBI(a) urbi::uobject_unarmorAndSend(#a)
#endif

/* urbi namespace starts */
namespace 
urbi {
  
  // Forward declarations and global scope structures
  class UObjectData;
  class UVar;
  class UObject;
  class UObjectHub;
  class baseURBIStarter;  
  class baseURBIStarterHub;  
  class UGenericCallback;
  class UTimerCallback;
  class UValue;
  class UVardata;

  // A few list and hashtable types
  typedef hash_map<string,list<UGenericCallback*> > UTable;
  typedef hash_map<string,list<UVar*> > UVarTable;
  typedef list<baseURBIStarter*> UStartlist;
  typedef list<baseURBIStarterHub*> UStartlistHub;
  typedef list<UTimerCallback*> UTimerTable;
  typedef list<UObject*> UObjectList;

  // The UReturn type
  typedef int UReturn;
  
  // Two singleton lists to handle the object and hubobject registration
  EXTERN_STATIC_INSTANCE(UStartlist, objectlist);
  EXTERN_STATIC_INSTANCE(UStartlistHub, objecthublist);

  // Lists and hashtables used
  extern UVarTable varmap;
  extern UTable functionmap;
  extern UTable eventmap;
  extern UTable eventendmap;
  extern UTable monitormap;
  extern UTable accessmap;  
  
  // Timer and update maps
  extern UTimerTable timermap;
  extern UTimerTable updatemap;

  // for remote mode
  extern void main(int argc, char *argv[]);

  // Notification mechanism
  void USync(UVar&);  
  void UNotifyChange(UVar&, int (*) ());
  void UNotifyChange(UVar&, int (*) (UVar&));
  void UNotifyChange(string, int (*) ());
  void UNotifyChange(string, int (*) (UVar&));
  void UNotifyAccess(UVar&, int (*) (UVar&));

  // *****************************************************************************
  // Global function of the urbi:: namespace to access kernel features

  void echo(const char * format, ... );
  UObjectHub* getUObjectHub(string); ///< retrieve a UObjectHub based on its name or return 0 if not found. 
  UObject* getUObject(string); ///< retrieve a UObject based on its name or return 0 if not found.

  /// Send URBI code (ghost connection in plugin mode, default connection in remote mode)
  void uobject_unarmorAndSend(const char * str);
  
  // *****************************************************************************
  // UValue and other related types
  
  enum UDataType {
    DATA_DOUBLE,
    DATA_STRING,
    DATA_BINARY,
    DATA_LIST,
    DATA_OBJECT,
    DATA_VOID
  };

  enum UBinaryType {
    BINARY_NONE,
    BINARY_UNKNOWN,
    BINARY_IMAGE,
    BINARY_SOUND
  };

  enum UImageFormat {
    IMAGE_RGB=1,     ///< RGB 24 bit/pixel
    IMAGE_YCbCr=2,   ///< YCbCr 24 bit/pixel
    IMAGE_JPEG=3,    ///< JPEG
    IMAGE_PPM=4,      ///< RGB with a PPM header    
    IMAGE_UNKNOWN 
  };

  enum USoundFormat {
    SOUND_RAW,
    SOUND_WAV,
    SOUND_MP3,
    SOUND_OGG,    
    SOUND_UNKNOWN
  };

  enum USoundSampleFormat {
    SAMPLE_SIGNED=1,
    SAMPLE_UNSIGNED=2
  };

  //WARNING: synchronize with blendNames in uvar.cc
  //! Blending mode
  enum UBlendType {
    UMIX=0,
    UADD,
    UDISCARD,
    UQUEUE,
    UCANCEL,
    UNORMAL
  };

  //WARNING: synchronize with propNames in uvar.cc
  /// URBI properties associated to a variable
  enum UProperty {
	  PROP_RANGEMIN=0,
	  PROP_RANGEMAX,
	  PROP_SPEEDMIN,
	  PROP_SPEEDMAX,
	  PROP_BLEND,
	  PROP_DELTA
  };
  //internal use: unparsed binary data
  class BinaryData {
    public:
      void * data;
      int size;
      BinaryData() {}
      BinaryData(void *d, int s):data(d), size(s) {}
  };


  ///Class encapsulating an image.
  class UImage {
    public:
      char                  *data;            ///< pointer to image data
      int                   size;             ///< image size in byte
      int                   width, height;    ///< size of the image
      UImageFormat          imageFormat;
  };

  ///Class encapsulating sound informations.
  class USound {
    public:
      char                  *data;            ///< pointer to sound data
      int                   size;             ///< total size in byte
      int                   channels;         ///< number of audio channels
      int                   rate;             ///< rate in Hertz
      int                   sampleSize;       ///< sample size in bit
      USoundFormat          soundFormat;      ///< format of the sound data
      USoundSampleFormat    sampleFormat;     ///< sample format

      bool operator ==(const USound &b) const {return !memcmp(this, &b, sizeof(USound));}
  };

  /// Class containing binary data sent by the server, that could not be furtehr interpreted.
  class UBinary {
    public:
      UBinaryType             type;
      union {
	struct {
	void                  *data;             ///< binary data
	int                   size;
	} common;
	UImage                image;
	USound                sound;
      };
      string                message;         ///< extra bin headers(everything after BIN <size> and before ';'

      UBinary();
      UBinary(const UBinary &b);  ///< deep copy constructor
	  explicit UBinary(const UImage &);
	  explicit UBinary(const USound &);
      UBinary & operator = (const UBinary &b); ///< deep copy
      void buildMessage(); ///< build message from structures
      string getMessage() const; ///< get message extracted from structures
      ~UBinary();  ///< Frees binary buffer
      int parse(const char * message, int pos, list<BinaryData> bins, list<BinaryData>::iterator &binpos);
  };

  class UList {
    public:
      vector<UValue *> array;
      UList();
      UList(const UList &b);
      UList & operator = (const UList &b);
      ~UList();
      UValue & operator [](int i) {return *array[i+offset];} 
      int size() {return array.size();}
      void setOffset(int n) { offset = n;};

    private:
      int offset;
  };

  class UNamedValue {
    public:
      UValue *val;
      string name;
      UNamedValue(string n, UValue *v):val(v),name(n) {}
      UNamedValue() {};
  };

  class UObjectStruct {
    public:
      string refName;
      vector<UNamedValue> array;
      UObjectStruct();
      UObjectStruct(const UObjectStruct &b);
      UObjectStruct & operator = (const UObjectStruct &b);
      ~UObjectStruct();
      UValue & operator [](string s);
      UNamedValue & operator [](int i) {return array[i];} 
      int size() {return array.size();}

  };

  class UValue {
    public:
      UDataType       type; 
      ufloat          val;
      union {
	string         *stringValue;
	UBinary        *binary;
	UList          *list;
	UObjectStruct  *object;
	void           *storage; // internal 
      };

      UValue();
      UValue(const UValue&);
      explicit UValue(ufloat doubleValue);
      explicit UValue(int intValue);
      explicit UValue(char * val);
      explicit UValue(const string &str);
      explicit UValue(const UBinary &b);
      explicit UValue(const UList & l);
      explicit UValue(const UObjectStruct &o);
	  explicit UValue(const USound &);
	  explicit UValue(const UImage &);
      operator ufloat() const;
      operator string() const;
      operator int() const {return (int)(ufloat)(*this);}
      operator bool() const {return (bool)(int)(ufloat)(*this);}
      operator UBinary() const; ///< deep copy
	  operator UList() const; ///< deep copy
      operator UImage() const; ///< ptr copy
      operator USound() const; ///< ptr copy
      UValue& operator=(const UValue&);

      ~UValue();  

      ///parse an uvalue in current message+pos, returns pos of end of match -pos of error if error
      int parse(char * message, int pos, std::list<BinaryData> bins, std::list<BinaryData>::iterator &binpos);
  };

  
  //! Provides easy access to variable properties
  class UProp 
  {
	  public:
	  void operator =(const UValue &v ) ;
	  void operator =(const double v ) ;
	  void operator =(const string & v ) ;
	  operator double() ;
	  operator string() ;
	  operator UValue() ;
	  
	  
	  UProp(UVar &owner, UProperty name):owner(owner),name(name){}
	  
	  private:
	  UVar & owner;
	  UProperty name;
	  //disable copy ctor and equal operator
	  UProp & operator =(const UProp &b);
	  UProp(const UProp &b);
  };
  
  
   //Helper macro to initialize UProps in UVar constructors
  #define VAR_PROP_INIT \
  rangemin(*this, PROP_RANGEMIN), \
  rangemax(*this, PROP_RANGEMAX), \
  speedmin(*this, PROP_SPEEDMIN), \
  speedmax(*this, PROP_SPEEDMAX), \
  delta(*this, PROP_DELTA), \
  blend(*this, PROP_BLEND)

  // *****************************************************************************
  //!UVar class definition
  class UVar
  {
  public:
    
  UVar() :VAR_PROP_INIT { name = "noname"; owned=false; vardata=0;};
  UVar(UVar& v) :VAR_PROP_INIT  {};
    UVar(const string&);
    UVar(const string&, const string&);
    UVar(UObject&, const string&);
    ~UVar();

    void init(const string&,const string&);
    void setOwned();

    void operator = ( ufloat );
    void operator = ( string );
    void operator = ( const UBinary &); 
    void operator = ( const UImage &i); ///< No data is copied in plugin mode
    void operator = ( const USound &s); ///< No data is copied in plugin mode


    operator int ();
    operator bool () {return (int)(*this);}
    operator UBinary ();   ///< deep copy
    operator UBinary *();  ///< deep copy, binary will have to be deleted by the user
    operator UImage (); ///< In plugin mode, gives direct access to the buffer, which may not be valid after the calling function returns. Changes to the other fields of the structure have no effect.
    operator USound(); ///< In plugin mode, gives direct access to the buffer, which may not be valid after the calling function returns. Changes to the other fields of the structure have no effect.
    operator ufloat ();
    operator string ();
   operator UList();  


   void requestValue(); ///< No effect in plugin mode. In remote mode, updates the value once asynchronously.

    //kernel operators
    ufloat& in();
    ufloat& out();
    //UBlendType blend();

    bool owned; ///< is the variable owned by the module?

	
	
	//Property accessors 
	
	UProp rangemin;
	UProp rangemax;
	UProp speedmin;
	UProp speedmax;
	UProp delta;
	UProp blend;
	
	
	UValue getProp(UProperty prop);
	void setProp(UProperty prop, const UValue &v);
	void setProp(UProperty prop, double v);
	void setProp(UProperty prop, const char * v);
	void setProp(UProperty prop, const string &v) {setProp(prop,v.c_str());}
    // internal
    void __update(UValue&);

  private:   
    UValue& val() { return value; }; ///< XXX only works in softdevice mode

    UVardata  *vardata; ///< pointer to internal data specifics
    void __init();	

    PRIVATE(string,name) ///< full name of the variable as seen in URBI      
    PRIVATE(UValue,value) ///< the variable value on the softdevice's side    
  };


 
  
  
  
  inline void UProp::operator =(const UValue &v ) {owner.setProp(name,v);}
  inline void UProp::operator =(const double v ) {owner.setProp(name,v);}
  inline void UProp::operator =(const string & v ) {owner.setProp(name,v);}
  inline UProp::operator double() {return (double)owner.getProp(name);}
  inline UProp::operator string()  {return (string)owner.getProp(name);}
  inline UProp::operator UValue() {return owner.getProp(name);}
	  
  // *****************************************************************************
  //! Function and Event storage mechanism
  /*! This heavily overloaded class is the only way in C++ to make life easy from the
      the interface user point's of view. 
  */
 
  class UGenericCallback
  {
  public:
    UGenericCallback(string type, string name, int size, UTable &t);
    UGenericCallback(string type, string name, UTable &t);
    virtual ~UGenericCallback();
    
    virtual UValue __evalcall(UList &param)  = 0;
    
    void   *storage; ////< used to store the UVar* pointeur for var monitoring
    ufloat period; ///< period of timers
    int    nbparam;

  private:
    string name; 
  };

  // *****************************************************************************
  //! Timer mechanism
  /*! This class stores a callback either as function or a class method
  */

  class UTimerCallback
  {
  public:
    UTimerCallback(ufloat period, UTimerTable &tt);
    virtual ~UTimerCallback();

    virtual void call() = 0;

    ufloat period;
    ufloat lastTimeCalled;
  };

  // UTimerCallback subclasses
  
  class UTimerCallbacknoobj : public UTimerCallback
  {
  public:
    UTimerCallbacknoobj(ufloat period, int (*fun) (), UTimerTable &tt): 
      UTimerCallback(period,tt), fun(fun) {};
    
    virtual void call() {
      (*fun)();      
    };
  private:
      int (*fun) ();
  };
  
  template <class T>
  class UTimerCallbackobj : public UTimerCallback
  {
  public:
    UTimerCallbackobj(ufloat period, T* obj, int (T::*fun) (), UTimerTable &tt): 
      UTimerCallback(period,tt), obj(obj), fun(fun) {};
    
    virtual void call() {
      ((*obj).*fun)();        
    };
  private:
      T* obj;
      int (T::*fun) ();
  };

  // *****************************************************************************
  //! Main UObject class definition
  class UObject
  {
  public:
    
    UObject(const string&);
    virtual ~UObject();

    template <class T> 
    void UNotifyChange (UVar& v, int (T::*fun) ()) { 
      createUCallback("var", (T*)this, fun, v.get_name(), monitormap);
    }

    template <class T>
    void UNotifyChange (UVar& v, int (T::*fun) (UVar&)) { 
      UGenericCallback* cb = createUCallback("var", (T*)this, fun, v.get_name(), monitormap);
      if (cb) cb->storage = (void*)(&v);
    }

    template <class T> 
    void UNotifyOnRequest (UVar& v, int (T::*fun) ()) { 
      createUCallback("var_onrequest", (T*)this, fun, v.get_name(), monitormap);
    }

    template <class T>
    void UNotifyOnRequest (UVar& v, int (T::*fun) (UVar&)) { 
      UGenericCallback* cb = createUCallback("var_onrequest", (T*)this, fun, v.get_name(), monitormap);
      if (cb) cb->storage = (void*)(&v);
    }


    template <class T> 
    void UNotifyChange (string name, int (T::*fun) ()) { 
      createUCallback("var", (T*)this, fun, name, monitormap);
    } 

    template <class T>
    void UNotifyChange (string name, int (T::*fun) (UVar&)) { 
      UGenericCallback* cb = createUCallback("var", (T*)this, fun, name, monitormap);
      if (cb) cb->storage = new UVar(name);
    }


    template <class T>
    void UNotifyAccess (UVar& v, int (T::*fun) (UVar&)) { 
      UGenericCallback* cb = createUCallback("varaccess", (T*)this, fun, v.get_name(), accessmap);
      if (cb) cb->storage = (void*)(&v);
    }

    template <class T>
    void USetTimer(ufloat t, int (T::*fun) ()) {
      new UTimerCallbackobj<T> (t,(T*)this, fun, timermap);
    }

    // We have to duplicate because of the above UNotifyChange which catches the
    // namespace on UObject instead of urbi.
    void UNotifyChange(UVar &v, int (*fun) ()) { urbi::UNotifyChange(v,fun); };
    void UNotifyChange(UVar &v, int (*fun) (UVar&)) { urbi::UNotifyChange(v,fun); };
    void UNotifyChange(string varname, int (*fun) ()) { urbi::UNotifyChange(varname,fun); };
    void UNotifyChange(string varname, int (*fun) (UVar&)) { urbi::UNotifyChange(varname,fun); };
  
    void UNotifyAccess(UVar &v, int (*fun) (UVar&)) { urbi::UNotifyAccess(v,fun); };

    string __name; ///< name of the object as seen in URBI
    string classname; ///< name of the class the object is derived from
    bool   derived; ///< true when the object has been newed by an urbi command 

    UObjectList members;
    UObjectHub  *objecthub; ///< the hub, if it exists

    void USetUpdate(ufloat);
    virtual int update() {return 0;}; 

    UVar        load; ///< the load attribute is standard and can be used to control the activity 
    		      ///< of the object
  
  private:
    UObjectData*  objectData; ///< pointer to a globalData structure specific to the 
                              ///< remote/plugin architectures who defines it.    
    ufloat period;
  };


  // *****************************************************************************
  //! Main UObjectHub class definition
  class UObjectHub
  {
  public:
    
    UObjectHub(const string&);
    virtual ~UObjectHub();

    void USetUpdate(ufloat);

    template <class T> 
    void UNotifyChange (UVar& v, int (T::*fun) ()) { 
      createUCallback("var", (T*)this, fun, v.get_name(), monitormap);
    }

    template <class T>
    void UNotifyChange (UVar& v, int (T::*fun) (UVar&)) { 
      UGenericCallback* cb = createUCallback("var", (T*)this, fun, v.get_name(), monitormap);
      if (cb) cb->storage = (void*)(&v);
    }

    template <class T> 
    void UNotifyChange (string name, int (T::*fun) ()) { 
      createUCallback("var", (T*)this, fun, name, monitormap);
    } 

    template <class T>
    void UNotifyChange (string name, int (T::*fun) (UVar&)) { 
      UGenericCallback* cb = createUCallback("var", (T*)this, fun, name, monitormap);
      if (cb) cb->storage = new UVar(name);
    }


    template <class T>
    void UNotifyAccess (UVar& v, int (T::*fun) (UVar&)) { 
      UGenericCallback* cb = createUCallback("varaccess", (T*)this, fun, v.get_name(), accessmap);
      if (cb) cb->storage = (void*)(&v);
    }

    template <class T>
    void USetTimer(ufloat t, int (T::*fun) ()) {
      new UTimerCallbackobj<T> (t, (T*)this,fun, timermap);      
    }

    void addMember(UObject* obj);
    virtual int update() {return 0;}
  

    UObjectList members;
  
    UObjectList* getSubClass(string);
 //   UObjectList* getAllSubClass(string); //TODO

  protected:
    int updateGlobal(); ///< this function calls update and the subclass update

    ufloat period;
    string name;
  };
    
  // *****************************************************************************
  //! Timer definition

  void USetTimer(ufloat t, int (*fun) ());
      
  template <class T>
    void USetTimer(ufloat t, T* obj, int (T::*fun) ()) {
      new UTimerCallbackobj<T> (t,obj,fun, timermap);
    }
  
  // *****************************************************************************
  // Casteurs

  
#ifndef _MSC_VER

	// generic caster  , second parameter used only to guess correct type
  template <class T>  T cast(UValue &v, T* type) { return (T)v; }

  // specializations 
  UVar& cast(UValue &val, UVar *var);
  UBinary cast(UValue &v, UBinary * b);
  UList cast(UValue &v, UList *l);
  UObjectStruct cast(UValue &v, UObjectStruct *o);
  
#else
  //something weird happens when overloading with a different return type, so define everythiong by hand
  
#define SETCAST(type) inline type cast(UValue &val, type *inu) { return (type)val;}

  SETCAST(int); SETCAST(ufloat); SETCAST(std::string);
  UVar& cast(UValue &val, UVar *var);
  UBinary cast(UValue &v, UBinary * b);
  UList cast(UValue &v, UList *l);
  UObjectStruct cast(UValue &v, UObjectStruct *o);
#endif

  // **************************************************************************	
  //! URBIStarter base class used to store heterogeneous template class objects in starterlist
  class baseURBIStarter
  {
  public:

    baseURBIStarter(string name) : name(name) {};
    virtual ~baseURBIStarter() { UObject* tokill = getUObject();
    	                         if (tokill) delete tokill;};

    virtual UObject* getUObject() = 0;

    virtual void init(string) =0; ///< Used to provide a wrapper to initialize objects in starterlist
    virtual void copy(string) = 0; ///< Used to provide a copy of a C++ object based on its name
    string name;
  };

  //! This is the class containing URBI starters
  /** A starter is a class whose job is to start an instance of a particular UObject subclass,
    * resulting in the initialization of this object (registration to the kernel)
    */
  template <class T> class URBIStarter : public baseURBIStarter
  {
  public:
    URBIStarter(string name, UStartlist& _slist) : baseURBIStarter(name)
    	{ slist = &_slist;
	  slist->push_back(dynamic_cast<baseURBIStarter*>(this)); 
	};
    virtual ~URBIStarter() {};

    virtual void copy(string objname) {
      	URBIStarter<T>* ustarter = new URBIStarter<T>(objname,*slist);
	ustarter->init(objname);
	dynamic_cast<UObject*>(object)->members.push_back(dynamic_cast<UObject*>(ustarter->object));
 	dynamic_cast<UObject*>(ustarter->object)->derived   = true;
        dynamic_cast<UObject*>(ustarter->object)->classname = 
	  dynamic_cast<UObject*>(object)->classname;
    };

    virtual UObject* getUObject() { 
      return dynamic_cast<UObject*>(object);
    }; ///< access to the object from the outside
    

  protected:
    virtual void init(string objname) { 
       object = new T(objname);
    }; ///< Called when the object is ready to be initialized    
    
    UStartlist  *slist;
    T*          object;
  };	

  // **************************************************************************	
  //! URBIStarter base class used to store heterogeneous template class objects in starterlist
  class baseURBIStarterHub
  {
  public:

    baseURBIStarterHub(string name) : name(name) {};
    virtual ~baseURBIStarterHub() {};

    virtual void init(string) = 0; ///< Used to provide a wrapper to initialize objects in starterlist
    virtual UObjectHub* getUObjectHub() = 0;
    string name;    
  };

  //! This is the class containing URBI starters
  /** A starter is a class whose job is to start an instance of a particular UObject subclass,
    * resulting in the initialization of this object (registration to the kernel)
    */
  template <class T> class URBIStarterHub : public baseURBIStarterHub
  {
  public:
    URBIStarterHub(string name, UStartlistHub& _slist) : baseURBIStarterHub(name)
    	{ slist = &_slist;
	  slist->push_back(dynamic_cast<baseURBIStarterHub*>(this)); 
	};
    virtual ~URBIStarterHub() { };

  protected:
    virtual void init(string objname) { 
       object = new T(objname); 
    }; ///< Called when the object is ready to be initialized    

    virtual UObjectHub* getUObjectHub() { 
      return dynamic_cast<UObjectHub*>(object);
    }; ///< access to the object from the outside
    
    UStartlistHub  *slist;
    T*                 object;
  };

  /**********************************************************/
  // This section is autogenerated. Not for humans eyes ;)
  /**********************************************************/
  
  template<typename T> class utrait {
  public:
    typedef T noref;
  };
  
#ifndef _MSC_VER
  
  template<typename T> class utrait<T&> {
  public:
    typedef T noref;
  };
  
#else  
  
  template<> class utrait<UVar&> {
  public:
    typedef UVar noref;
  };
  
#endif

%%%% 0 16

  // non void, object methods
  
  template <class OBJ, class R%%, class P% %%>
    class UCallback%N% : public UGenericCallback
  {
  public:
    UCallback%N%(string type, OBJ* obj, R (OBJ::*fun) (%%%,% P% %%), string funname, UTable &t): 
      UGenericCallback(type, funname,%N%, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)(%%%,% cast(param[% - 1], (typename utrait<P%>::noref *)0) %%));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) (%%%,% P% %%); 
  };
   
  // void, object methods

  template <class OBJ%%, class P% %%>
    class UCallbackvoid%N% : public UGenericCallback
  {
  public:
    UCallbackvoid%N%(string type, OBJ* obj, void (OBJ::*fun) (%%%,% P% %%), string funname, UTable &t): 
      UGenericCallback(type, funname,%N%, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)(%%%,% cast(param[% - 1], (typename utrait<P%>::noref *)0) %%);
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) (%%%,% P% %%);
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ%%, class P% %%>
    class UCallbacknotifyend%N% : public UGenericCallback
  {
  public:
    UCallbacknotifyend%N%(string type, OBJ* obj, void (OBJ::*fun) (%%%,% P% %%), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,%N%, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R%%, class P% %%>
    class UCallbackGlobal%N% : public UGenericCallback
  {
  public:
    UCallbackGlobal%N%(string type, R (*fun) (%%%,% P% %%), string funname, UTable &t): 
      UGenericCallback(type, funname,%N%, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)(%%%,% cast(param[% - 1], (typename utrait<P%>::noref *)0) %%));
    };
  private:      
      R (*fun) (%%%,% P% %%);
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R%%, class P% %%> 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) (%%%,% P% %%), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback%N%<OBJ,R%%, P% %%> (type,obj,fun,funname,t));
  }

  template <class OBJ%%, class P% %%> 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) (%%%,% P% %%), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid%N%<OBJ%%, P% %%> (type,obj,fun,funname,t));
  }
   
  template <class R%%, class P% %%> 
  UGenericCallback* createUCallback(string type, R (*fun) (%%%,% P% %%), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal%N%<R%%, P% %%> (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ%%, class P% %%> 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) (%%%,% P% %%), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend%N%<OBJ%%, P% %%> (type,obj,fun,end,funname,t));
  }
   

%%%%


  // Special case for void, standard functions
  
  class UCallbackGlobalvoid0 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid0(string type, void (*fun) (), string funname, UTable &t): 
      UGenericCallback(type, funname,0, t), fun(fun) {};
    virtual UValue __evalcall(UList &) {
      (*fun)();
      return UValue();
    };
  private:
      void (*fun) ();
  };

  UGenericCallback* createUCallback(string type, void (*fun) (), string funname,UTable &t);

%%%% 1 16 
  template <%%%,% class P% %%>
    class UCallbackGlobalvoid%N% : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid%N%(string type, void (*fun) (%%%,% P% %%), string funname, UTable &t): 
      UGenericCallback(type, funname,%N%, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)(%%%,% cast(param[% - 1], (typename utrait<P%>::noref *)0) %%);
      return UValue();
    };
  private:
      void (*fun) (%%%,% P% %%);
  };

  template <%%%,% class P% %%> 
  UGenericCallback* createUCallback(string type, void (*fun) (%%%,% P% %%), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid%N%<%%%,% P% %%> (type,fun,funname,t));
  }

%%%%
    
  

} // end namespace urbi

std::ostream & operator <<(std::ostream &s, const urbi::UValue &v);

#endif


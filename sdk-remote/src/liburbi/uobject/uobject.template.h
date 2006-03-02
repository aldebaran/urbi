/*! \file uobject.h
 *******************************************************************************

 File: uobject.h\n
 Definition of the UObject class and necessary related classes.

 This file is part of LIBURBI\n
 (c) Jean-Christophe Baillie, 2004-2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http:////<www.urbiforge.com

 **************************************************************************** */

#ifndef UOBJECT_H_DEFINED
#define UOBJECT_H_DEFINED

#include <string>
#include <list>
#include <hash_map.h>
//#include <iostream> // should not be there, remove after debug
//#include <uclient.h>
using namespace std;

extern const bool NOTIFYNEW; 

// A quick hack to be able to use hash_map with string easily
namespace __gnu_cxx {
  template<> struct hash< std::string > {  
    size_t operator()( const std::string& x ) const
    { return hash< const char* >()( x.c_str() );}
  };
}


// This macro is here to make life easier
// Simply use: UStarter(myUObjectType) and the rest will be taken care of.
#define UStart(x) urbi::URBIStarter<x> x ## ____URBI_object(string(#x))

// Thess macro are here to make life easier
#define UVarInit(obj,x) x.init(name,#x)
#define UFunctionInit(obj,x)  createUCallback("function", this,(&obj::x),name+"."+string(#x),functionmap)
#define UEventInit(obj,x)     createUCallback("event",    this,(&obj::x),name+"."+string(#x),eventmap)
#define UNotifyEnd(obj,x,fun) createUCallback("eventend", this,(&obj::x),(&obj::fun),name+"."+string(#x),eventendmap)


// defines a variable and it's associated accessors
#define PRIVATE(vartype,varname) private: vartype varname;public: vartype get_ ## varname \
  () { return varname; }; void set_ ## varname (vartype& ____value) { varname = ____value; };private:

/* urbi namespace starts */
namespace 
urbi {
  
  // Forward declarations and global scope structures
  class UObjectData;
  class UVar;
  class UObject;
  class baseURBIStarter;  
  class UGenericCallback;
  class UValue;
  class UVardata;

  // For homogeneity of the code, UFunction and UEvent are nothing more than UValue's
  typedef UValue UFunction;
  typedef void UEvent;
  typedef hash_map<string,list<UGenericCallback*> > UTable;
  typedef hash_map<string,list<UVar*> > UVarTable;

  
  extern list<baseURBIStarter*> objectlist;
  extern UVarTable varmap;
  extern UTable functionmap;
  extern UTable eventmap;
  extern UTable eventendmap;
  extern UTable monitormap;

  extern void main(int argc, char *argv[]);

  void UMonitor(UVar&);  
  void UMonitor(UVar&, int (*) ());
  void UMonitor(UVar&, int (*) (UVar&));
  void UMonitor(string, int (*) ());
  void UMonitor(string, int (*) (UVar&));

  void echo(const char * format, ... );

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
    IMAGE_PPM=4      ///< RGB with a PPM header
  };


  enum USoundFormat {
    SOUND_RAW,
    SOUND_WAV,
    SOUND_MP3,
    SOUND_OGG
  };

  enum USoundSampleFormat {
    SAMPLE_SIGNED=1,
    SAMPLE_UNSIGNED=2
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


      // USound() : data(0), size(0), channels(1), rate(16000), sampleSize(2), sampleFormat(SAMPLE_SIGNED) {}
      bool operator ==(const USound &b) const {return !memcmp(this, &b, sizeof(USound));}
  };



  /// Class containing binary data sent by the server, that could not be furtehr interpreted.
  class UBinary {
    public:
      UBinaryType             type;
      union {
	void                  *data;             ///< binary data
	UImage                image;
	USound                sound;
      };
      string                message;         ///< message as sent by the server
      int                   size;


      UBinary();
      UBinary(const UBinary &b);
      UBinary & operator = (const UBinary &b);
      ~UBinary();
      int parse(char * message, int pos, list<BinaryData> bins, list<BinaryData>::iterator &binpos);
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
      UNamedValue(string n, UValue *v):name(n), val(v) {}
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
      UFloat          val;
      union {
	string         *stringValue;
	UBinary        *binary;
	UList          *list;
	UObjectStruct  *object;
	void           *storage; // internal 
      };

      UValue();
      UValue(const UValue&);
      explicit UValue(UFloat doubleValue);
      explicit UValue(int intValue);
      explicit UValue(char * val);
      explicit UValue(const string &str);
      explicit UValue(const UBinary &b);
      explicit UValue(const UList & l);
      explicit UValue(const UObjectStruct &o);
      operator UFloat();
      operator string();
      operator int() {return (int)(UFloat)(*this);}

      UValue& operator=(const UValue&);

      ~UValue();  

      ///parse an uvalue in current message+pos, returns pos of end of match -pos of error if error
      int parse(char * message, int pos, std::list<BinaryData> bins, std::list<BinaryData>::iterator &binpos);
  };




  // **************************************************************************	
  //! URBIStarter base class used to store heterogeneous template class objects in starterlist
  class baseURBIStarter
  {
  public:

    baseURBIStarter(string name) : name(name) {};
    virtual ~baseURBIStarter() {};

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
    URBIStarter(string name) : baseURBIStarter(name)
    	{ objectlist.push_back(dynamic_cast<baseURBIStarter*>(this)); };
    virtual ~URBIStarter() { };

    virtual void copy(string objname) {
      	new URBIStarter<T>(objname);
	init(objname);
    };

  protected:
    virtual void init(string objname)    { new T(objname); }; ///< Called when the object is ready to be initialized
  };	

  
  // *****************************************************************************
  //!UVar class definition
  class UVar
  {
  public:
    
    UVar() { name = "noname";};
    UVar(UVar& v) {};
    UVar(string);
    UVar(string,string);
    UVar(UObject&, string);
    ~UVar();

    void init(string, string);

    void operator = ( float );
    void operator = ( string );
    operator int () { return ((int)value); };
    operator UFloat () { return ((UFloat)value); };
    operator string () { return ((string)value); };
  
    UValue& val() { return value; };

    // internal
    void __update(UValue&);

  private:    
    UVarData  *vardata; ///< pointer to internal data specifics
    void __init();	

    PRIVATE(string,name); ///< full name of the variable as seen in URBI      
    PRIVATE(UValue,value); ///< the variable value on the softdevice's side    
  };


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
    ~UGenericCallback();
    
    virtual UValue __evalcall(UList &param)  = 0;
    
    void   *storage; ////< used to store the UVar* pointeur for var monitoring

  private:
    string name; 
  };

  

  // *****************************************************************************
  //! Main UObject class definition
  class UObject
  {
  public:
    
    UObject(const string&, bool);
    ~UObject();

    template <class T> 
    void UMonitor(UVar& v, int (T::*fun) ()) { 
      createUCallback("var", (T*)this, fun, v.get_name(), monitormap);
    }; 

    template <class T>
    void UMonitor(UVar& v, int (T::*fun) (UVar&)) { 
      UGenericCallback* cb = createUCallback("var", (T*)this, fun, v.get_name(), monitormap);
      if (cb) cb->storage = (void*)(&v);
    };

    // We have to duplicate because of the above UMonitor which catches the
    // namespace on UObject instead of urbi.
    void UMonitor(UVar &v) { urbi::UMonitor(v); };
    void UMonitor(UVar &v, int (*fun) ()) { urbi::UMonitor(v,fun); };
    void UMonitor(UVar &v, int (*fun) (UVar&)) { urbi::UMonitor(v,fun); };
    void UMonitor(string varname, int (*fun) ()) { urbi::UMonitor(varname,fun); };
    void UMonitor(string varname, int (*fun) (UVar&)) { urbi::UMonitor(varname,fun); };
  
    string name; ///< name of the object as seen in URBI
    
  private:
    UObjectData*  objectData; ///< pointer to a globalData structure specific to the 
                              ///< module/plugin architectures who defines it.
    bool       notifynew; ///< is the object notified when a 'new' command is done on the URBI side?
  };

  // generic caster  
  template <class T>  T cast(UValue &v) { return (T)v; }

  // specializations 
  template <> UVar& cast(UValue &v);
  template <> UBinary cast(UValue &v);
  template <> UList cast(UValue &v);
  template <> UObjectStruct cast(UValue &v);

  /**********************************************************/
  // This section is autogenerated. Not for humans eyes ;)
  /**********************************************************/
  
%%%% 0 16

  // non void, object methods
  
  template <class OBJ, class R%%, class P% %%>
    class UCallback%N% : public UGenericCallback
  {
  public:
    UCallback%N%(string type, OBJ* obj, R (OBJ::*fun) (%%%,% P% %%), string funname, UTable &t): 
      UGenericCallback(type, funname,%N%, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)(%%%,% cast<P%>(param[% - 1]) %%));
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
      ((*obj).*fun)(%%%,% cast<P%>(param[% - 1]) %%);
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
      return UValue((*fun)(%%%,% cast<P%>(param[% - 1]) %%));
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
      (*fun)(%%%,% cast<P%>(param[% - 1]) %%);
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


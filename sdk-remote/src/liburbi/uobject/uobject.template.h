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

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#ifndef UOBJECT_H_DEFINED
#define UOBJECT_H_DEFINED

#include <string>
#include <list>
#include <hash_map.h>
#include <iostream> // should not be there, remove after debug
#include <uclient.h>
using namespace std;


// USed by some classes and defined somewhere else in liburbi. This avoids including
// multiple .h that are not actually used by the programmer
class UValue;

// A quick hack to be able to use hash_map with string easily
namespace __gnu_cxx {
  template<> struct hash< std::string > {  
    size_t operator()( const std::string& x ) const
    { return hash< const char* >()( x.c_str() );}
  };
}


// This macro is here to make life easier
// Simply use: UStarter(myUObjectType) and the rest will be taken care of.
#define UStart(x) URBI::URBIStarter<x> x ## ____URBI_object

// Thess macro are here to make life easier
#define UVarInit(obj,x) x.init(#obj,#x)
#define UFunctionInit(obj,x)  createUCallback("function", this,(&obj::x),string(#obj)+"."+string(#x),functionmap)
#define UEventInit(obj,x)     createUCallback("event", this,(&obj::x),string(#obj)+"."+string(#x),eventmap)

// defines a variable and it's associated accessors
#define PRIVATE(vartype,varname) private: vartype varname;public: vartype get_ ## varname \
  () { return varname; }; void set_ ## varname (vartype& ____value) { varname = ____value; };private:

/* URBI namespace starts */
namespace 
URBI {
  
  // Forward declarations and global scope structures
  class UObjectData;
  class UVar;
  class UObject;
  class baseURBIStarter;  
  class UGenericCallback;

  // For homogeneity of the code, UFunction and UEvent are nothing more than UValue's
  typedef UValue UFunction;
  typedef void UEvent;
  typedef hash_map<string,list<UGenericCallback*> > UTable;
  typedef hash_map<string,list<UVar*> > UVarTable;

  
  extern list<baseURBIStarter*> objectlist;
  extern UVarTable varmap;
  extern UTable functionmap;
  extern UTable eventmap;
  extern UTable monitormap;

  extern void main(int argc, char *argv[]);

  void UMonitor(UVar&);  
  void UMonitor(UVar&, int (*) ());
  void UMonitor(UVar&, int (*) (UVar&));


  // **************************************************************************	
  //! URBIStarter base class used to store heterogeneous template class objects in starterlist
  class baseURBIStarter
  {
  public:

    baseURBIStarter() {};
    virtual ~baseURBIStarter() {};

    virtual void init() =0; //< Used to provide a wrapper to initialize objects in starterlist
  };

  //! This is the class containing URBI starters
  /** A starter is a class whose job is to start an instance of a particular UObject subclass,
    * resulting in the initialization of this object (registration to the kernel)
    */
  template <class T> class URBIStarter : public baseURBIStarter
  {
  public:
    URBIStarter()          { objectlist.push_back(dynamic_cast<baseURBIStarter*>(this)); };
    virtual ~URBIStarter() { };

  protected:
    virtual void init()    { new T; }; //< Called when the object is ready to be initialized
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
    operator double () { return ((double)value); };
    operator string () { return ((string)value); };
  

    // internal
    void __update(UValue&);

  private:    

    PRIVATE(string,name); //< full name of the variable as seen in URBI  
    
    PRIVATE(UValue,value);
    void __init();
  };



  // *****************************************************************************
  //! Main UObject class definition
  class UObject
  {
  public:
    
    UObject(const string&);
    ~UObject();

    template <class T> 
    void UMonitor(UVar& v, int (T::*fun) ()) { 
      createUCallback("var", (T*)this, fun, v.get_name(), monitormap);
    }; 

    template <class T>
    void UMonitor(UVar& v, int (T::*fun) (UVar&)) { 
      createUCallback("var", (T*)this, fun, v.get_name(), monitormap);
    };

    void UMonitor(UVar &v) { URBI::UMonitor(v); };
    void UMonitor(UVar &v, int (*fun) ()) { URBI::UMonitor(v,fun); };
    void UMonitor(UVar &v, int (*fun) (UVar&)) { URBI::UMonitor(v,fun); };

  private:
    UObjectData*  objectData; //< pointer to a globalData structure specific to the 
                              //< module/plugin architectures who defines it.
     
    PRIVATE(string,name); //< name of the object as seen in URBI
  };

  // generic caster
 // template <class T>
 // T cast(UValue &v) { return (T)v; }
  
  template <class T>
  T cast(UValue &v) { return (T)v; }

  // specialization for UVar
  template <>
  UVar& cast(UValue &v);


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
    
    virtual UValue __evalcall(UValue *param)  = 0;

  private:
    string name;    
  };

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
    virtual UValue __evalcall(UValue *param) {
      return UValue((obj->*fun)(%%%,% cast<P%>(param[% - 1]) %%));
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
    virtual UValue __evalcall(UValue *param) {
      (obj->*fun)(%%%,% cast<P%>(param[% - 1]) %%);
      return (UValue());
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) (%%%,% P% %%);
  };

  // non void, standard function
  
  template <class R%%, class P% %%>
    class UCallbackGlobal%N% : public UGenericCallback
  {
  public:
    UCallbackGlobal%N%(string type, R (*fun) (%%%,% P% %%), string funname, UTable &t): 
      UGenericCallback(type, funname,%N%, t), fun(fun) {};
    virtual UValue __evalcall(UValue *param) {
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
%%%%


  // Special case for void, standard functions
  
  class UCallbackGlobalvoid0 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid0(string type, void (*fun) (), string funname, UTable &t): 
      UGenericCallback(type, funname,0, t), fun(fun) {};
    virtual UValue __evalcall(UValue *param) {
      (*fun)();
      return (UValue());
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
    virtual UValue __evalcall(UValue *param) {
      (*fun)(%%%,% cast<P%>(param[% - 1]) %%);
      return (UValue());
    };
  private:
      void (*fun) (%%%,% P% %%);
  };

  template <%%%,% class P% %%> 
  UGenericCallback* createUCallback(string type, void (*fun) (%%%,% P% %%), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid%N%<%%%,% P% %%> (type,fun,funname,t));
  }

%%%%
    
  

} // end namespace URBI

using namespace URBI;

#endif


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
#define UEventInit(obj,x)     createUCallback("event",    this,(&obj::x),string(#obj)+"."+string(#x),eventmap)
#define UNotifyEnd(obj,x,fun) createUCallback("eventend", this,(&obj::x),(&obj::fun),string(#obj)+"."+string(#x),eventendmap)


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
  extern UTable eventendmap;
  extern UTable monitormap;

  extern void main(int argc, char *argv[]);

  void UMonitor(UVar&);  
  void UMonitor(UVar&, int (*) ());
  void UMonitor(UVar&, int (*) (UVar&));
  void UMonitor(string, int (*) ());
  void UMonitor(string, int (*) (UVar&));



  // **************************************************************************	
  //! URBIStarter base class used to store heterogeneous template class objects in starterlist
  class baseURBIStarter
  {
  public:

    baseURBIStarter() {};
    virtual ~baseURBIStarter() {};

    virtual void init() =0; ///< Used to provide a wrapper to initialize objects in starterlist
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
    virtual void init()    { new T; }; ///< Called when the object is ready to be initialized
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
  
    UValue& val() { return value; };

    // internal
    void __update(UValue&);

  private:    

    PRIVATE(string,name); ////< full name of the variable as seen in URBI      
    PRIVATE(UValue,value);
    void __init();
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
    
    UObject(const string&);
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

    void UMonitor(UVar &v) { URBI::UMonitor(v); };
    void UMonitor(UVar &v, int (*fun) ()) { URBI::UMonitor(v,fun); };
    void UMonitor(UVar &v, int (*fun) (UVar&)) { URBI::UMonitor(v,fun); };
    void UMonitor(string varname, int (*fun) ()) { URBI::UMonitor(varname,fun); };
    void UMonitor(string varname, int (*fun) (UVar&)) { URBI::UMonitor(varname,fun); };


  private:
    UObjectData*  objectData; ///< pointer to a globalData structure specific to the 
                              ///< module/plugin architectures who defines it.
     
    PRIVATE(string,name); ///< name of the object as seen in URBI
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
  

  // non void, object methods
  
  template <class OBJ, class R>
    class UCallback0 : public UGenericCallback
  {
  public:
    UCallback0(string type, OBJ* obj, R (OBJ::*fun) (), string funname, UTable &t): 
      UGenericCallback(type, funname,0, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)());
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) (); 
  };
   
  // void, object methods

  template <class OBJ>
    class UCallbackvoid0 : public UGenericCallback
  {
  public:
    UCallbackvoid0(string type, OBJ* obj, void (OBJ::*fun) (), string funname, UTable &t): 
      UGenericCallback(type, funname,0, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ>
    class UCallbacknotifyend0 : public UGenericCallback
  {
  public:
    UCallbacknotifyend0(string type, OBJ* obj, void (OBJ::*fun) (), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,0, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R>
    class UCallbackGlobal0 : public UGenericCallback
  {
  public:
    UCallbackGlobal0(string type, R (*fun) (), string funname, UTable &t): 
      UGenericCallback(type, funname,0, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)());
    };
  private:      
      R (*fun) ();
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R> 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) (), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback0<OBJ,R> (type,obj,fun,funname,t));
  }

  template <class OBJ> 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) (), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid0<OBJ> (type,obj,fun,funname,t));
  }
   
  template <class R> 
  UGenericCallback* createUCallback(string type, R (*fun) (), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal0<R> (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ> 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) (), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend0<OBJ> (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 >
    class UCallback1 : public UGenericCallback
  {
  public:
    UCallback1(string type, OBJ* obj, R (OBJ::*fun) ( P1 ), string funname, UTable &t): 
      UGenericCallback(type, funname,1, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 >
    class UCallbackvoid1 : public UGenericCallback
  {
  public:
    UCallbackvoid1(string type, OBJ* obj, void (OBJ::*fun) ( P1 ), string funname, UTable &t): 
      UGenericCallback(type, funname,1, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 >
    class UCallbacknotifyend1 : public UGenericCallback
  {
  public:
    UCallbacknotifyend1(string type, OBJ* obj, void (OBJ::*fun) ( P1 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,1, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 >
    class UCallbackGlobal1 : public UGenericCallback
  {
  public:
    UCallbackGlobal1(string type, R (*fun) ( P1 ), string funname, UTable &t): 
      UGenericCallback(type, funname,1, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) ));
    };
  private:      
      R (*fun) ( P1 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback1<OBJ,R, P1 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid1<OBJ, P1 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal1<R, P1 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend1<OBJ, P1 > (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 , class P2 >
    class UCallback2 : public UGenericCallback
  {
  public:
    UCallback2(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 ), string funname, UTable &t): 
      UGenericCallback(type, funname,2, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 , P2 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 , class P2 >
    class UCallbackvoid2 : public UGenericCallback
  {
  public:
    UCallbackvoid2(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 ), string funname, UTable &t): 
      UGenericCallback(type, funname,2, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 , P2 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 , class P2 >
    class UCallbacknotifyend2 : public UGenericCallback
  {
  public:
    UCallbacknotifyend2(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,2, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 , class P2 >
    class UCallbackGlobal2 : public UGenericCallback
  {
  public:
    UCallbackGlobal2(string type, R (*fun) ( P1 , P2 ), string funname, UTable &t): 
      UGenericCallback(type, funname,2, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) ));
    };
  private:      
      R (*fun) ( P1 , P2 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 , class P2 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback2<OBJ,R, P1 , P2 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 , class P2 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid2<OBJ, P1 , P2 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 , class P2 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 , P2 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal2<R, P1 , P2 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 , class P2 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend2<OBJ, P1 , P2 > (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 , class P2 , class P3 >
    class UCallback3 : public UGenericCallback
  {
  public:
    UCallback3(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 ), string funname, UTable &t): 
      UGenericCallback(type, funname,3, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 , P2 , P3 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 , class P2 , class P3 >
    class UCallbackvoid3 : public UGenericCallback
  {
  public:
    UCallbackvoid3(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 ), string funname, UTable &t): 
      UGenericCallback(type, funname,3, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 , P2 , P3 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 , class P2 , class P3 >
    class UCallbacknotifyend3 : public UGenericCallback
  {
  public:
    UCallbacknotifyend3(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,3, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 , class P2 , class P3 >
    class UCallbackGlobal3 : public UGenericCallback
  {
  public:
    UCallbackGlobal3(string type, R (*fun) ( P1 , P2 , P3 ), string funname, UTable &t): 
      UGenericCallback(type, funname,3, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) ));
    };
  private:      
      R (*fun) ( P1 , P2 , P3 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 , class P2 , class P3 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback3<OBJ,R, P1 , P2 , P3 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 , class P2 , class P3 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid3<OBJ, P1 , P2 , P3 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 , class P2 , class P3 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 , P2 , P3 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal3<R, P1 , P2 , P3 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 , class P2 , class P3 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend3<OBJ, P1 , P2 , P3 > (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 >
    class UCallback4 : public UGenericCallback
  {
  public:
    UCallback4(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 ), string funname, UTable &t): 
      UGenericCallback(type, funname,4, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 , P2 , P3 , P4 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 , class P2 , class P3 , class P4 >
    class UCallbackvoid4 : public UGenericCallback
  {
  public:
    UCallbackvoid4(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 ), string funname, UTable &t): 
      UGenericCallback(type, funname,4, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 , P2 , P3 , P4 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 , class P2 , class P3 , class P4 >
    class UCallbacknotifyend4 : public UGenericCallback
  {
  public:
    UCallbacknotifyend4(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,4, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 , class P2 , class P3 , class P4 >
    class UCallbackGlobal4 : public UGenericCallback
  {
  public:
    UCallbackGlobal4(string type, R (*fun) ( P1 , P2 , P3 , P4 ), string funname, UTable &t): 
      UGenericCallback(type, funname,4, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) ));
    };
  private:      
      R (*fun) ( P1 , P2 , P3 , P4 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback4<OBJ,R, P1 , P2 , P3 , P4 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 , class P2 , class P3 , class P4 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid4<OBJ, P1 , P2 , P3 , P4 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 , class P2 , class P3 , class P4 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 , P2 , P3 , P4 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal4<R, P1 , P2 , P3 , P4 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 , class P2 , class P3 , class P4 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend4<OBJ, P1 , P2 , P3 , P4 > (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 >
    class UCallback5 : public UGenericCallback
  {
  public:
    UCallback5(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 ), string funname, UTable &t): 
      UGenericCallback(type, funname,5, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 >
    class UCallbackvoid5 : public UGenericCallback
  {
  public:
    UCallbackvoid5(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 ), string funname, UTable &t): 
      UGenericCallback(type, funname,5, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 >
    class UCallbacknotifyend5 : public UGenericCallback
  {
  public:
    UCallbacknotifyend5(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,5, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 >
    class UCallbackGlobal5 : public UGenericCallback
  {
  public:
    UCallbackGlobal5(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 ), string funname, UTable &t): 
      UGenericCallback(type, funname,5, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) ));
    };
  private:      
      R (*fun) ( P1 , P2 , P3 , P4 , P5 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback5<OBJ,R, P1 , P2 , P3 , P4 , P5 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid5<OBJ, P1 , P2 , P3 , P4 , P5 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal5<R, P1 , P2 , P3 , P4 , P5 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend5<OBJ, P1 , P2 , P3 , P4 , P5 > (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 >
    class UCallback6 : public UGenericCallback
  {
  public:
    UCallback6(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 ), string funname, UTable &t): 
      UGenericCallback(type, funname,6, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 >
    class UCallbackvoid6 : public UGenericCallback
  {
  public:
    UCallbackvoid6(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 ), string funname, UTable &t): 
      UGenericCallback(type, funname,6, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 >
    class UCallbacknotifyend6 : public UGenericCallback
  {
  public:
    UCallbacknotifyend6(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,6, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 >
    class UCallbackGlobal6 : public UGenericCallback
  {
  public:
    UCallbackGlobal6(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 ), string funname, UTable &t): 
      UGenericCallback(type, funname,6, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) ));
    };
  private:      
      R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback6<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid6<OBJ, P1 , P2 , P3 , P4 , P5 , P6 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal6<R, P1 , P2 , P3 , P4 , P5 , P6 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend6<OBJ, P1 , P2 , P3 , P4 , P5 , P6 > (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 >
    class UCallback7 : public UGenericCallback
  {
  public:
    UCallback7(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 ), string funname, UTable &t): 
      UGenericCallback(type, funname,7, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 >
    class UCallbackvoid7 : public UGenericCallback
  {
  public:
    UCallbackvoid7(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 ), string funname, UTable &t): 
      UGenericCallback(type, funname,7, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 >
    class UCallbacknotifyend7 : public UGenericCallback
  {
  public:
    UCallbacknotifyend7(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,7, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 >
    class UCallbackGlobal7 : public UGenericCallback
  {
  public:
    UCallbackGlobal7(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 ), string funname, UTable &t): 
      UGenericCallback(type, funname,7, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) ));
    };
  private:      
      R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback7<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid7<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal7<R, P1 , P2 , P3 , P4 , P5 , P6 , P7 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend7<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 > (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 >
    class UCallback8 : public UGenericCallback
  {
  public:
    UCallback8(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 ), string funname, UTable &t): 
      UGenericCallback(type, funname,8, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 >
    class UCallbackvoid8 : public UGenericCallback
  {
  public:
    UCallbackvoid8(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 ), string funname, UTable &t): 
      UGenericCallback(type, funname,8, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 >
    class UCallbacknotifyend8 : public UGenericCallback
  {
  public:
    UCallbacknotifyend8(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,8, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 >
    class UCallbackGlobal8 : public UGenericCallback
  {
  public:
    UCallbackGlobal8(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 ), string funname, UTable &t): 
      UGenericCallback(type, funname,8, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) ));
    };
  private:      
      R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback8<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid8<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal8<R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend8<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 > (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 >
    class UCallback9 : public UGenericCallback
  {
  public:
    UCallback9(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 ), string funname, UTable &t): 
      UGenericCallback(type, funname,9, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 >
    class UCallbackvoid9 : public UGenericCallback
  {
  public:
    UCallbackvoid9(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 ), string funname, UTable &t): 
      UGenericCallback(type, funname,9, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 >
    class UCallbacknotifyend9 : public UGenericCallback
  {
  public:
    UCallbacknotifyend9(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,9, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 >
    class UCallbackGlobal9 : public UGenericCallback
  {
  public:
    UCallbackGlobal9(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 ), string funname, UTable &t): 
      UGenericCallback(type, funname,9, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) ));
    };
  private:      
      R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback9<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid9<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal9<R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend9<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 > (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 >
    class UCallback10 : public UGenericCallback
  {
  public:
    UCallback10(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 ), string funname, UTable &t): 
      UGenericCallback(type, funname,10, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 >
    class UCallbackvoid10 : public UGenericCallback
  {
  public:
    UCallbackvoid10(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 ), string funname, UTable &t): 
      UGenericCallback(type, funname,10, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 >
    class UCallbacknotifyend10 : public UGenericCallback
  {
  public:
    UCallbacknotifyend10(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,10, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 >
    class UCallbackGlobal10 : public UGenericCallback
  {
  public:
    UCallbackGlobal10(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 ), string funname, UTable &t): 
      UGenericCallback(type, funname,10, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) ));
    };
  private:      
      R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback10<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid10<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal10<R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend10<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 > (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 >
    class UCallback11 : public UGenericCallback
  {
  public:
    UCallback11(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 ), string funname, UTable &t): 
      UGenericCallback(type, funname,11, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 >
    class UCallbackvoid11 : public UGenericCallback
  {
  public:
    UCallbackvoid11(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 ), string funname, UTable &t): 
      UGenericCallback(type, funname,11, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 >
    class UCallbacknotifyend11 : public UGenericCallback
  {
  public:
    UCallbacknotifyend11(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,11, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 >
    class UCallbackGlobal11 : public UGenericCallback
  {
  public:
    UCallbackGlobal11(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 ), string funname, UTable &t): 
      UGenericCallback(type, funname,11, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) ));
    };
  private:      
      R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback11<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid11<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal11<R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend11<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 > (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 >
    class UCallback12 : public UGenericCallback
  {
  public:
    UCallback12(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 ), string funname, UTable &t): 
      UGenericCallback(type, funname,12, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 >
    class UCallbackvoid12 : public UGenericCallback
  {
  public:
    UCallbackvoid12(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 ), string funname, UTable &t): 
      UGenericCallback(type, funname,12, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 >
    class UCallbacknotifyend12 : public UGenericCallback
  {
  public:
    UCallbacknotifyend12(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,12, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 >
    class UCallbackGlobal12 : public UGenericCallback
  {
  public:
    UCallbackGlobal12(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 ), string funname, UTable &t): 
      UGenericCallback(type, funname,12, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) ));
    };
  private:      
      R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback12<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid12<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal12<R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend12<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 > (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 >
    class UCallback13 : public UGenericCallback
  {
  public:
    UCallback13(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 ), string funname, UTable &t): 
      UGenericCallback(type, funname,13, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 >
    class UCallbackvoid13 : public UGenericCallback
  {
  public:
    UCallbackvoid13(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 ), string funname, UTable &t): 
      UGenericCallback(type, funname,13, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 >
    class UCallbacknotifyend13 : public UGenericCallback
  {
  public:
    UCallbacknotifyend13(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,13, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 >
    class UCallbackGlobal13 : public UGenericCallback
  {
  public:
    UCallbackGlobal13(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 ), string funname, UTable &t): 
      UGenericCallback(type, funname,13, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) ));
    };
  private:      
      R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback13<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid13<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal13<R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend13<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 > (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 >
    class UCallback14 : public UGenericCallback
  {
  public:
    UCallback14(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 ), string funname, UTable &t): 
      UGenericCallback(type, funname,14, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) , cast<P14>(param[14 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 >
    class UCallbackvoid14 : public UGenericCallback
  {
  public:
    UCallbackvoid14(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 ), string funname, UTable &t): 
      UGenericCallback(type, funname,14, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) , cast<P14>(param[14 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 >
    class UCallbacknotifyend14 : public UGenericCallback
  {
  public:
    UCallbacknotifyend14(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,14, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 >
    class UCallbackGlobal14 : public UGenericCallback
  {
  public:
    UCallbackGlobal14(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 ), string funname, UTable &t): 
      UGenericCallback(type, funname,14, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) , cast<P14>(param[14 - 1]) ));
    };
  private:      
      R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback14<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid14<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal14<R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend14<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 > (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 >
    class UCallback15 : public UGenericCallback
  {
  public:
    UCallback15(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 ), string funname, UTable &t): 
      UGenericCallback(type, funname,15, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) , cast<P14>(param[14 - 1]) , cast<P15>(param[15 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 >
    class UCallbackvoid15 : public UGenericCallback
  {
  public:
    UCallbackvoid15(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 ), string funname, UTable &t): 
      UGenericCallback(type, funname,15, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) , cast<P14>(param[14 - 1]) , cast<P15>(param[15 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 >
    class UCallbacknotifyend15 : public UGenericCallback
  {
  public:
    UCallbacknotifyend15(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,15, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 >
    class UCallbackGlobal15 : public UGenericCallback
  {
  public:
    UCallbackGlobal15(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 ), string funname, UTable &t): 
      UGenericCallback(type, funname,15, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) , cast<P14>(param[14 - 1]) , cast<P15>(param[15 - 1]) ));
    };
  private:      
      R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback15<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid15<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal15<R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend15<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 > (type,obj,fun,end,funname,t));
  }
   


  // non void, object methods
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 , class P16 >
    class UCallback16 : public UGenericCallback
  {
  public:
    UCallback16(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 ), string funname, UTable &t): 
      UGenericCallback(type, funname,16, t), obj(obj), fun(fun) {};
    virtual UValue __evalcall(UList& param) {
      return UValue(( (*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) , cast<P14>(param[14 - 1]) , cast<P15>(param[15 - 1]) , cast<P16>(param[16 - 1]) ));
    };
  private:
    OBJ* obj;
    R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 ); 
  };
   
  // void, object methods

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 , class P16 >
    class UCallbackvoid16 : public UGenericCallback
  {
  public:
    UCallbackvoid16(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 ), string funname, UTable &t): 
      UGenericCallback(type, funname,16, t), obj(obj), fun(fun) {};
    
    virtual UValue __evalcall(UList &param) {
      ((*obj).*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) , cast<P14>(param[14 - 1]) , cast<P15>(param[15 - 1]) , cast<P16>(param[16 - 1]) );
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 );
  };
  
  // void, object methods : special case for notifyend event callbacks

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 , class P16 >
    class UCallbacknotifyend16 : public UGenericCallback
  {
  public:
    UCallbacknotifyend16(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 ), void (OBJ::*end)(),string funname, UTable &t): 
      UGenericCallback(type, funname,16, t), obj(obj), fun(end) {};
    
    virtual UValue __evalcall(UList &) {
      ((*obj).*fun)();
      return UValue();
    };
  private:
      OBJ* obj;
      void (OBJ::*fun) ();
  };


  // non void, standard function
  
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 , class P16 >
    class UCallbackGlobal16 : public UGenericCallback
  {
  public:
    UCallbackGlobal16(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 ), string funname, UTable &t): 
      UGenericCallback(type, funname,16, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      return UValue((*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) , cast<P14>(param[14 - 1]) , cast<P15>(param[15 - 1]) , cast<P16>(param[16 - 1]) ));
    };
  private:      
      R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 );
  };
  
  
  // callback creation for obj void & non void + standard function non void
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 , class P16 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallback16<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 > (type,obj,fun,funname,t));
  }

  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 , class P16 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackvoid16<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 > (type,obj,fun,funname,t));
  }
   
  template <class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 , class P16 > 
  UGenericCallback* createUCallback(string type, R (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobal16<R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 > (type,fun,funname,t));
  }

  // special case for eventend notification
  template <class OBJ, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 , class P16 > 
  UGenericCallback* createUCallback(string type, OBJ* obj, void (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 ), void (OBJ::*end)(), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbacknotifyend16<OBJ, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 > (type,obj,fun,end,funname,t));
  }
   



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

  template < class P1 >
    class UCallbackGlobalvoid1 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid1(string type, void (*fun) ( P1 ), string funname, UTable &t): 
      UGenericCallback(type, funname,1, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 );
  };

  template < class P1 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid1< P1 > (type,fun,funname,t));
  }

  template < class P1 , class P2 >
    class UCallbackGlobalvoid2 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid2(string type, void (*fun) ( P1 , P2 ), string funname, UTable &t): 
      UGenericCallback(type, funname,2, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 , P2 );
  };

  template < class P1 , class P2 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 , P2 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid2< P1 , P2 > (type,fun,funname,t));
  }

  template < class P1 , class P2 , class P3 >
    class UCallbackGlobalvoid3 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid3(string type, void (*fun) ( P1 , P2 , P3 ), string funname, UTable &t): 
      UGenericCallback(type, funname,3, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 , P2 , P3 );
  };

  template < class P1 , class P2 , class P3 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 , P2 , P3 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid3< P1 , P2 , P3 > (type,fun,funname,t));
  }

  template < class P1 , class P2 , class P3 , class P4 >
    class UCallbackGlobalvoid4 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid4(string type, void (*fun) ( P1 , P2 , P3 , P4 ), string funname, UTable &t): 
      UGenericCallback(type, funname,4, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 , P2 , P3 , P4 );
  };

  template < class P1 , class P2 , class P3 , class P4 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 , P2 , P3 , P4 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid4< P1 , P2 , P3 , P4 > (type,fun,funname,t));
  }

  template < class P1 , class P2 , class P3 , class P4 , class P5 >
    class UCallbackGlobalvoid5 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid5(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 ), string funname, UTable &t): 
      UGenericCallback(type, funname,5, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 , P2 , P3 , P4 , P5 );
  };

  template < class P1 , class P2 , class P3 , class P4 , class P5 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid5< P1 , P2 , P3 , P4 , P5 > (type,fun,funname,t));
  }

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 >
    class UCallbackGlobalvoid6 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid6(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 ), string funname, UTable &t): 
      UGenericCallback(type, funname,6, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 );
  };

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid6< P1 , P2 , P3 , P4 , P5 , P6 > (type,fun,funname,t));
  }

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 >
    class UCallbackGlobalvoid7 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid7(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 ), string funname, UTable &t): 
      UGenericCallback(type, funname,7, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 );
  };

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid7< P1 , P2 , P3 , P4 , P5 , P6 , P7 > (type,fun,funname,t));
  }

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 >
    class UCallbackGlobalvoid8 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid8(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 ), string funname, UTable &t): 
      UGenericCallback(type, funname,8, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 );
  };

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid8< P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 > (type,fun,funname,t));
  }

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 >
    class UCallbackGlobalvoid9 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid9(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 ), string funname, UTable &t): 
      UGenericCallback(type, funname,9, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 );
  };

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid9< P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 > (type,fun,funname,t));
  }

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 >
    class UCallbackGlobalvoid10 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid10(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 ), string funname, UTable &t): 
      UGenericCallback(type, funname,10, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 );
  };

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid10< P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 > (type,fun,funname,t));
  }

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 >
    class UCallbackGlobalvoid11 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid11(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 ), string funname, UTable &t): 
      UGenericCallback(type, funname,11, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 );
  };

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid11< P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 > (type,fun,funname,t));
  }

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 >
    class UCallbackGlobalvoid12 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid12(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 ), string funname, UTable &t): 
      UGenericCallback(type, funname,12, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 );
  };

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid12< P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 > (type,fun,funname,t));
  }

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 >
    class UCallbackGlobalvoid13 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid13(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 ), string funname, UTable &t): 
      UGenericCallback(type, funname,13, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 );
  };

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid13< P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 > (type,fun,funname,t));
  }

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 >
    class UCallbackGlobalvoid14 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid14(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 ), string funname, UTable &t): 
      UGenericCallback(type, funname,14, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) , cast<P14>(param[14 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 );
  };

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid14< P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 > (type,fun,funname,t));
  }

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 >
    class UCallbackGlobalvoid15 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid15(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 ), string funname, UTable &t): 
      UGenericCallback(type, funname,15, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) , cast<P14>(param[14 - 1]) , cast<P15>(param[15 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 );
  };

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid15< P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 > (type,fun,funname,t));
  }

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 , class P16 >
    class UCallbackGlobalvoid16 : public UGenericCallback
  {
  public:
    UCallbackGlobalvoid16(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 ), string funname, UTable &t): 
      UGenericCallback(type, funname,16, t), fun(fun) {};
    virtual UValue __evalcall(UList &param) {
      (*fun)( cast<P1>(param[1 - 1]) , cast<P2>(param[2 - 1]) , cast<P3>(param[3 - 1]) , cast<P4>(param[4 - 1]) , cast<P5>(param[5 - 1]) , cast<P6>(param[6 - 1]) , cast<P7>(param[7 - 1]) , cast<P8>(param[8 - 1]) , cast<P9>(param[9 - 1]) , cast<P10>(param[10 - 1]) , cast<P11>(param[11 - 1]) , cast<P12>(param[12 - 1]) , cast<P13>(param[13 - 1]) , cast<P14>(param[14 - 1]) , cast<P15>(param[15 - 1]) , cast<P16>(param[16 - 1]) );
      return UValue();
    };
  private:
      void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 );
  };

  template < class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 , class P16 > 
  UGenericCallback* createUCallback(string type, void (*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 ), string funname,UTable &t) {
    return ((UGenericCallback*) new UCallbackGlobalvoid16< P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 , P16 > (type,fun,funname,t));
  }

    
  

} // end namespace URBI

using namespace URBI;

#endif


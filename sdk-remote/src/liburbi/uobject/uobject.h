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
#define UVarInit(__x,x) x.init(#x)
#define UFunctionInit(obj,x)  new UFunctionInitializer(#obj, #x,createUCallback(this,&obj::x))
#define UEventInit(obj,x) int zss = 7;

// defines a variable and it's associated accessors
#define PRIVATE(vartype,varname) private: vartype varname;public: vartype get_ ## varname \
  () { return varname; }; void set_ ## varname (vartype& value) { varname = value; }

/* URBI namespace starts */
namespace 
URBI {
  
  // For homogeneity of the code, UFunction and UEvent are nothing more than UValue's
  typedef UValue UFunction;
  typedef void UEvent;

  // Forward declarations and global scope structures
  class UObjectData;
  class UVar;
  class baseURBIStarter;
  class UFunctionInitializer;
  class UGenericCallback;
  
  extern list<baseURBIStarter*> objectlist;
  extern hash_map<string,UVar*> varmap;
  extern hash_map<string,UFunctionInitializer*> functionmap;

  extern void main(int argc, char *argv[]);

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
  //! Main UObject class definition
  class UObject
  {
  public:
    
    UObject(const string&);
    ~UObject();

  private:
    UObjectData*  objectData; //< pointer to a globalData structure specific to the 
                              //< module/plugin architectures who defines it.
     
    PRIVATE(string,name); //< name of the object as seen in URBI
  };

  
  // *****************************************************************************
  //!UVar class definition
  class UVar
  {
  public:
    
    UVar() {};
    UVar(UValue);
    UVar(string, bool = false);
    UVar(string,string, bool = false);
    UVar(UObject&, string, bool = false);
    ~UVar();

    void init(string);

    void operator = ( float );
    void operator = ( string );
    operator int () { return ((int)value); };
    operator double () { return ((double)value); };
    operator string () { return ((string)value); };
    
  private:    

    PRIVATE(string,name); //< full name of the variable as seen in URBI  
    
    UValue value;
    void __init(bool);
    void __update(UValue&);
  };

  
  // *****************************************************************************
  //! Function and Event storage mechanism
  /*! This heavily overloaded class is the only way in C++ to make life easy from the
      the interface user point's of view. Close yours eyes, it's ugly :)
  */
 
  class UFunctionInitializer
  {
  public:
    UFunctionInitializer(string objname, string funname, UGenericCallback* cb){
       name = objname+"."+funname;
       functionmap[name] = this;
       this->cb = cb;
    };
    ~UFunctionInitializer() {};
    

    virtual UValue __evalfunction(int nbparam, UValue *param) {    
     
      //return ret;
    };
    
  private:
    string     name;
    UGenericCallback *cb;
  };


  class UGenericCallback
  {
  public:
    ~UGenericCallback() {};

    virtual UValue __evalcall(UValue *param)  = 0;
  };

  template <class OBJ, class R>
    class UCallback0 : public UGenericCallback
  {
  public:
    UCallback0(OBJ* obj, R (OBJ::*fun) ()): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)());
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ();
  };
  
  template <class OBJ, class R> 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ()) {
    return ((UGenericCallback*) new UCallback0<OBJ,R> (obj,fun));
  }
  template <class OBJ, class R, class P1 >
    class UCallback1 : public UGenericCallback
  {
  public:
    UCallback1(OBJ* obj, R (OBJ::*fun) ( P1 )): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)( param[1] ));
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ( P1 );
  };
  
  template <class OBJ, class R, class P1 > 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ( P1 )) {
    return ((UGenericCallback*) new UCallback1<OBJ,R, P1 > (obj,fun));
  }
  template <class OBJ, class R, class P1 , class P2 >
    class UCallback2 : public UGenericCallback
  {
  public:
    UCallback2(OBJ* obj, R (OBJ::*fun) ( P1 , P2 )): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)( param[1] , param[2] ));
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ( P1 , P2 );
  };
  
  template <class OBJ, class R, class P1 , class P2 > 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ( P1 , P2 )) {
    return ((UGenericCallback*) new UCallback2<OBJ,R, P1 , P2 > (obj,fun));
  }
  template <class OBJ, class R, class P1 , class P2 , class P3 >
    class UCallback3 : public UGenericCallback
  {
  public:
    UCallback3(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 )): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)( param[1] , param[2] , param[3] ));
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ( P1 , P2 , P3 );
  };
  
  template <class OBJ, class R, class P1 , class P2 , class P3 > 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 )) {
    return ((UGenericCallback*) new UCallback3<OBJ,R, P1 , P2 , P3 > (obj,fun));
  }
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 >
    class UCallback4 : public UGenericCallback
  {
  public:
    UCallback4(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 )): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)( param[1] , param[2] , param[3] , param[4] ));
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ( P1 , P2 , P3 , P4 );
  };
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 > 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 )) {
    return ((UGenericCallback*) new UCallback4<OBJ,R, P1 , P2 , P3 , P4 > (obj,fun));
  }
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 >
    class UCallback5 : public UGenericCallback
  {
  public:
    UCallback5(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 )): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)( param[1] , param[2] , param[3] , param[4] , param[5] ));
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 );
  };
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 > 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 )) {
    return ((UGenericCallback*) new UCallback5<OBJ,R, P1 , P2 , P3 , P4 , P5 > (obj,fun));
  }
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 >
    class UCallback6 : public UGenericCallback
  {
  public:
    UCallback6(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 )): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)( param[1] , param[2] , param[3] , param[4] , param[5] , param[6] ));
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 );
  };
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 > 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 )) {
    return ((UGenericCallback*) new UCallback6<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 > (obj,fun));
  }
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 >
    class UCallback7 : public UGenericCallback
  {
  public:
    UCallback7(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 )): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)( param[1] , param[2] , param[3] , param[4] , param[5] , param[6] , param[7] ));
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 );
  };
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 > 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 )) {
    return ((UGenericCallback*) new UCallback7<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 > (obj,fun));
  }
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 >
    class UCallback8 : public UGenericCallback
  {
  public:
    UCallback8(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 )): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)( param[1] , param[2] , param[3] , param[4] , param[5] , param[6] , param[7] , param[8] ));
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 );
  };
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 > 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 )) {
    return ((UGenericCallback*) new UCallback8<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 > (obj,fun));
  }
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 >
    class UCallback9 : public UGenericCallback
  {
  public:
    UCallback9(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 )): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)( param[1] , param[2] , param[3] , param[4] , param[5] , param[6] , param[7] , param[8] , param[9] ));
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 );
  };
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 > 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 )) {
    return ((UGenericCallback*) new UCallback9<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 > (obj,fun));
  }
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 >
    class UCallback10 : public UGenericCallback
  {
  public:
    UCallback10(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 )): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)( param[1] , param[2] , param[3] , param[4] , param[5] , param[6] , param[7] , param[8] , param[9] , param[10] ));
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 );
  };
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 > 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 )) {
    return ((UGenericCallback*) new UCallback10<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 > (obj,fun));
  }
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 >
    class UCallback11 : public UGenericCallback
  {
  public:
    UCallback11(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 )): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)( param[1] , param[2] , param[3] , param[4] , param[5] , param[6] , param[7] , param[8] , param[9] , param[10] , param[11] ));
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 );
  };
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 > 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 )) {
    return ((UGenericCallback*) new UCallback11<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 > (obj,fun));
  }
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 >
    class UCallback12 : public UGenericCallback
  {
  public:
    UCallback12(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 )): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)( param[1] , param[2] , param[3] , param[4] , param[5] , param[6] , param[7] , param[8] , param[9] , param[10] , param[11] , param[12] ));
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 );
  };
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 > 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 )) {
    return ((UGenericCallback*) new UCallback12<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 > (obj,fun));
  }
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 >
    class UCallback13 : public UGenericCallback
  {
  public:
    UCallback13(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 )): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)( param[1] , param[2] , param[3] , param[4] , param[5] , param[6] , param[7] , param[8] , param[9] , param[10] , param[11] , param[12] , param[13] ));
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 );
  };
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 > 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 )) {
    return ((UGenericCallback*) new UCallback13<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 > (obj,fun));
  }
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 >
    class UCallback14 : public UGenericCallback
  {
  public:
    UCallback14(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 )): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)( param[1] , param[2] , param[3] , param[4] , param[5] , param[6] , param[7] , param[8] , param[9] , param[10] , param[11] , param[12] , param[13] , param[14] ));
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 );
  };
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 > 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 )) {
    return ((UGenericCallback*) new UCallback14<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 > (obj,fun));
  }
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 >
    class UCallback15 : public UGenericCallback
  {
  public:
    UCallback15(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 )): 
      obj(obj), 
      fun(fun) {};
      virtual UValue __evalcall(UValue *param) {
        return UValue((obj->*fun)( param[1] , param[2] , param[3] , param[4] , param[5] , param[6] , param[7] , param[8] , param[9] , param[10] , param[11] , param[12] , param[13] , param[14] , param[15] ));
      };
  private:
      OBJ* obj;
      R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 );
  };
  
  template <class OBJ, class R, class P1 , class P2 , class P3 , class P4 , class P5 , class P6 , class P7 , class P8 , class P9 , class P10 , class P11 , class P12 , class P13 , class P14 , class P15 > 
    UGenericCallback* createUCallback(OBJ* obj, R (OBJ::*fun) ( P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 )) {
    return ((UGenericCallback*) new UCallback15<OBJ,R, P1 , P2 , P3 , P4 , P5 , P6 , P7 , P8 , P9 , P10 , P11 , P12 , P13 , P14 , P15 > (obj,fun));
  }
    
 
} // end namespace URBI

using namespace URBI;

#endif


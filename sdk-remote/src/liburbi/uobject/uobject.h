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
#define UFunctionInit(obj,x)  new UFunctionInitializer<obj>(#obj, #x,this,&obj::x)
#define UEventInit(obj,x) int zss = 7;

// defines a variable and it's associated accessors
#define PRIVATE(vartype,varname) private: vartype varname;public: vartype get_ ## varname \
  () { return varname; }; void set_ ## varname (vartype& value) { varname = value; }

/* URBI namespace starts */
namespace 
URBI {
  
  // For homogeneity of the code, UFunction and UEvent are nothing more than UValue's
  typedef UValue UFunction;
  typedef UValue UEvent;

  // Forward declarations and global scope structures
  class UObjectData;
  class UVar;
  class baseURBIStarter;
  class baseUFunctionInitializer;
  
  extern list<baseURBIStarter*> objectlist;
  extern hash_map<string,UVar*> varmap;
  extern hash_map<string,baseUFunctionInitializer*> functionmap;

  extern void URBIMain(int argc, char *argv[]);

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

  class baseUFunctionInitializer {
  public:
    baseUFunctionInitializer() {};
    ~baseUFunctionInitializer() {};
    
    virtual UValue __evalfunction(int,UValue*)=0;
  };

  template <class T> 
  class UFunctionInitializer : public baseUFunctionInitializer
  {
  public:
    UFunctionInitializer(string, string, T*, UValue (T::*) ());
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&));
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&,UVar&));        
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&,UVar&,UVar&));
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&));
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&));
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&));
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&));
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&));
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&));
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&));
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&));
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&));
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&));
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&));
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&));
    UFunctionInitializer(string, string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&));
    
    ~UFunctionInitializer() {};

    virtual UValue __evalfunction(int nbparam,UValue *param) {    
      UVar *vars[nbparam];
      UValue ret;
      for (int i=0;i<nbparam;i++) 
        vars[i] = new UVar(param[i]);//I don't like that, but would need a fix in liburbi to improve

      switch (nbparam) {
      case 0:  ret = (uobj->*fun0)(); break;
      case 1:  ret = (uobj->*fun1)(*vars[0]); break;
      case 2:  ret = (uobj->*fun2)(*vars[0],*vars[1]); break;
      case 3:  ret = (uobj->*fun3)(*vars[0],*vars[1],*vars[2]); break;
      case 4:  ret = (uobj->*fun4)(*vars[0],*vars[1],*vars[2],*vars[3]); break;
      case 5:  ret = (uobj->*fun5)(*vars[0],*vars[1],*vars[2],*vars[3],*vars[4]); break;
      case 6:  ret = (uobj->*fun6)(*vars[0],*vars[1],*vars[2],*vars[3],*vars[4],*vars[5]); break;
      case 7:  ret = (uobj->*fun7)(*vars[0],*vars[1],*vars[2],*vars[3],*vars[4],*vars[5],*vars[6]); break;
      case 8:  ret = (uobj->*fun8)(*vars[0],*vars[1],*vars[2],*vars[3],*vars[4],*vars[5],*vars[6],*vars[7]); break;
      case 9:  ret = (uobj->*fun9)(*vars[0],*vars[1],*vars[2],*vars[3],*vars[4],*vars[5],*vars[6],*vars[7],*vars[8]); break;
      case 10: ret = (uobj->*fun10)(*vars[0],*vars[1],*vars[2],*vars[3],*vars[4],*vars[5],*vars[6],*vars[7],*vars[8],*vars[9]); break;
      case 11: ret = (uobj->*fun11)(*vars[0],*vars[1],*vars[2],*vars[3],*vars[4],*vars[5],*vars[6],*vars[7],*vars[8],*vars[9],*vars[10]); break;
      case 12: ret = (uobj->*fun12)(*vars[0],*vars[1],*vars[2],*vars[3],*vars[4],*vars[5],*vars[6],*vars[7],*vars[8],*vars[9],*vars[10],*vars[11]); break;
      case 13: ret = (uobj->*fun13)(*vars[0],*vars[1],*vars[2],*vars[3],*vars[4],*vars[5],*vars[6],*vars[7],*vars[8],*vars[9],*vars[10],*vars[11],*vars[12]); break;
      case 14: ret = (uobj->*fun14)(*vars[0],*vars[1],*vars[2],*vars[3],*vars[4],*vars[5],*vars[6],*vars[7],*vars[8],*vars[9],*vars[10],*vars[11],*vars[12],*vars[13]); break;
      case 15: ret = (uobj->*fun15)(*vars[0],*vars[1],*vars[2],*vars[3],*vars[4],*vars[5],*vars[6],*vars[7],*vars[8],*vars[9],*vars[10],*vars[11],*vars[12],*vars[13],*vars[14]); break;
      case 16: ret = (uobj->*fun16)(*vars[0],*vars[1],*vars[2],*vars[3],*vars[4],*vars[5],*vars[6],*vars[7],*vars[8],*vars[9],*vars[10],*vars[11],*vars[12],*vars[13],*vars[14],*vars[15]); break;
      };
      
      for (int i=0;i<nbparam;i++)
        delete vars[i];
      return ret;
    };
    
  private:
    T*   uobj;
    string name;    
    UValue (T::*fun0)  ();
    UValue (T::*fun1)  (UVar&);
    UValue (T::*fun2)  (UVar&,UVar&);       
    UValue (T::*fun3)  (UVar&,UVar&,UVar&);
    UValue (T::*fun4)  (UVar&,UVar&,UVar&,UVar&);
    UValue (T::*fun5)  (UVar&,UVar&,UVar&,UVar&,UVar&);
    UValue (T::*fun6)  (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&);
    UValue (T::*fun7)  (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&);
    UValue (T::*fun8)  (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&);
    UValue (T::*fun9)  (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&);
    UValue (T::*fun10) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&);
    UValue (T::*fun11) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&);
    UValue (T::*fun12) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&);
    UValue (T::*fun13) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&);
    UValue (T::*fun14) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&);
    UValue (T::*fun15) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&);
    UValue (T::*fun16) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&);

    void __init(string objname, string funname, T* uobj) {
      this->uobj = uobj;
      name = objname+"."+funname;
      functionmap[name] = (baseUFunctionInitializer*)this;
    };
  };
  
  
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname,  T* uobj, UValue (T::*fun) ()) {      
    fun0 = fun; __init(objname, funname, uobj);
  }
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname,  T* uobj, UValue (T::*fun) (UVar&)) {
    fun1 = fun; __init(objname, funname, uobj);
  }
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&)) {   
    fun2 = fun; __init(objname, funname, uobj);
  }       
  template <class T> 
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&,UVar&)) {
    fun3 = fun; __init(objname, funname, uobj);
  }
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&,UVar&,UVar&)) {
    fun4 = fun; __init(objname, funname, uobj);
  }
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&,UVar&,UVar&,UVar&)) {
    fun5 = fun; __init(objname, funname, uobj);
  }
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {
    fun6 = fun; __init(objname, funname, uobj);
  }
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {
    fun7 = fun; __init(objname, funname, uobj);
  }
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {
    fun8 = fun; __init(objname, funname, uobj);
  }
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {
    fun9 = fun; __init(objname, funname, uobj);
  }
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {
    fun10 = fun; __init(objname, funname, uobj);
  }
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {
    fun11 = fun; __init(objname, funname, uobj);
  }
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {
    fun12 = fun; __init(objname, funname, uobj);
  }
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {
    fun13 = fun; __init(objname, funname, uobj);
  }
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {
    fun14 = fun; __init(objname, funname, uobj);
  }
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {
    fun15 = fun; __init(objname, funname, uobj);
  }
  template <class T>
    UFunctionInitializer<T>::UFunctionInitializer(string objname, string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {
    fun16 = fun; __init(objname, funname, uobj);
  }

} // end namespace URBI

using namespace URBI;

#endif


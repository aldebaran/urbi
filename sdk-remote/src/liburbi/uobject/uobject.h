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
#define UFunctionInit(obj,x) UFunctionInitializer<obj> __##x(#x,this,&obj::x)
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

    int toto;
    int titi;

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
    
  private:    

    PRIVATE(string,name); //< full name of the variable as seen in URBI  
    
    UValue *value;
    void __init(bool);
    void __update(UValue&);
  };

  
  // *****************************************************************************
  //! Function and Event storage mechanism

  class baseUFunctionInitializer {
  public:
    baseUFunctionInitializer() {};
    ~baseUFunctionInitializer() {};

    virtual UValue evalfunction(int,UValue*)=0;
  };

  template <class T> 
  class UFunctionInitializer : public baseUFunctionInitializer
  {
  public:
    UFunctionInitializer(string, T*, UValue (T::*) ());
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&));
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&,UVar&));
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&,UVar&,UVar&)) {};
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&)) {};
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&)) {};
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {};
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {};
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {};
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {};
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {};
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {};
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {};
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {};
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {};
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {};
    UFunctionInitializer(string, T*, UValue (T::*) (UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&,UVar&)) {};
    ~UFunctionInitializer() {};

    virtual UValue evalfunction(int nbparam,UValue *param) {            
      UVar *vars[nbparam];
      UValue ret;

      for (int i=0;i<nbparam;i++) 
        vars[i] = new UVar(param[i]);//I don't like that, but would need a fix in liburbi to improve

      switch (nbparam) {
      case 0: ret = (uobj->*fun0)();
      case 1: ret = (uobj->*fun1)(*vars[0]);
      case 2: ret = (uobj->*fun2)(*vars[0],*vars[1]);
      };
      
      for (int i=0;i<nbparam;i++)
        delete vars[i];
      return ret;
    };
    
  private:
    T*   uobj;
    UValue (T::*fun0) ();
    UValue (T::*fun1) (UVar&);
    UValue (T::*fun2) (UVar&,UVar&);
  };
  
  
  template <class T>
  UFunctionInitializer<T>::UFunctionInitializer(string funname,  T* uobj, UValue (T::*fun) ())
  {    
    this->uobj = uobj;
    fun0 = fun;
    functionmap[funname] = (baseUFunctionInitializer*)this;
  }
  template <class T>
  UFunctionInitializer<T>::UFunctionInitializer(string funname,  T* uobj, UValue (T::*fun) (UVar&))
  {    
    this->uobj = uobj;
    fun1 = fun;
    functionmap[funname] = (baseUFunctionInitializer*)this;
  }
  template <class T>
  UFunctionInitializer<T>::UFunctionInitializer(string funname, T* uobj, UValue (T::*fun) (UVar&,UVar&))
  {    
    this->uobj = uobj;
    fun2 = fun;
    functionmap[funname] = (baseUFunctionInitializer*)this;
  }
  
} // end namespace URBI

using namespace URBI;

#endif


/*! \file uobject.cc
 *******************************************************************************
 
 File: uobject.cc\n
 Implementation of the UObject class.

 This file is part of 
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#include <stdarg.h>
#include <stdio.h>
#include <list>
#include "userver.h"
#include "uobject.h"
using namespace urbi;
using namespace std;

#define LIBURBIDEBUG

//! Global definition of the starterlist
namespace urbi {
  
  UObject* lastUObject;

  STATIC_INSTANCE(UStartlist, objectlist);
  STATIC_INSTANCE(UStartlistHub, objecthublist);

  const string externalModuleTag = "__ExternalMessage__";

  UVarTable varmap;
  UTable functionmap;
  UTable monitormap;
  UTable accessmap;
  UTable eventmap;
  UTable eventendmap;

  UTimerTable timermap;
  UTimerTable updatemap;

  
  UVar& cast(UValue &v, UVar *) {
    return (*((UVar*)v.storage));    
  };

  UBinary cast(UValue &v, UBinary *) {
	if (v.type != DATA_BINARY) {
	  return UBinary();
	}
	return UBinary(*v.binary);
  }
  
  UList cast(UValue &v, UList *) {
	if (v.type != DATA_LIST)
	  return UList();
	return UList(*v.list);
  }
  
  UObjectStruct cast(UValue &v, UObjectStruct*) {
	if (v.type != DATA_OBJECT)
	  return UObjectStruct();
	return UObjectStruct(*v.object);
  }
  
}

using namespace urbi;

void urbi::main(int argc, char *argv[]) {} // no effect here

// **************************************************************************	
//! UGenericCallback constructor.
UGenericCallback::UGenericCallback(string type, string name, int size,  UTable &t) : 
  name(name) , storage(0)
{
  nbparam = size;
  
  if ((type == "function") || (type== "event") || (type=="eventend")) {};
  
  t[this->name].push_back(this);
    
  if (type == "var") {
    
    HMvariabletab::iterator it = ::urbiserver->variabletab.find(name.c_str());
    if (it == ::urbiserver->variabletab.end()) {
	  
      UVariable *variable = new UVariable(name.c_str(), new ::UValue());
      if (variable) variable->internalBinder.push_back(this);
    }
    else 
      it->second->internalBinder.push_back(this);
  }
   
  if (type == "varaccess") {
    
    HMvariabletab::iterator it = ::urbiserver->variabletab.find(name.c_str());
    if (it == ::urbiserver->variabletab.end()) {
	  
      UVariable *variable = new UVariable(name.c_str(), new ::UValue());
      if (variable) variable->internalAccessBinder.push_back(this);
    }
    else 
      it->second->internalAccessBinder.push_back(this);
  }

  
  // Note pour la suite:
  // ucommand.cc:3342 => mise en place de bindings. C'est là que se trouve
  // l'info pour traiter les callback function/event
  //
  // Voir aussi uvariable.cc:354 pour gérer la mise à jour d'une variable =>
  // rien à faire du côté de UVar qui a un pointeur sur UVariable de toute
  // façon, mais il faut appeller les callback des variables monitorées
  //
  // Ensuite, chercher les /// EXTERNAL et gérer la version internal au même
  // endroit.
  //
  // A priori, l'affectation dans les UVar est reglée (UVar x;  x=42)
  //
  // Penser à sortir les méthodes de parsing des object UBinary et urbi::UValue,
  // ça n'a rien à faire dans le kernel.
  //
  // Mettre en place des castings pour les UVar et les urbi::UValue

  /* Principe de l'object UObjectHub:

  L'diée est d'avoir un object UObjectHub perso du genre:

  class myhub : UObjectHub {
    virtual void callmeAfterAllCallbacks();
  }  

  callmeAfterAllCallbacks est appellé après l'appel concommitant de n callback
  de UObject appartenant au hub. Ca veut dire que si deux objets sont callbackés
  en même temps (pour ne pas dire au même tour de boucle), cette fonction est
  appellé après que les deux cb ai été executé. On peut mettre là le code qui
  envoit le message OPENR pour bouger les moteurs par exemple. Et on peut savoir
  quels sont les objets qui ont callbacké et préparer le tableau en fonction.
  Voir comment le hub et les objets partagent des données.

  L'autre idée est de permettre de définir au niveau de la couche OS-specific la
  notion de timer. Un hub peut demander à être reveillé à intervalles réguliers.
  Comme par hasard, dans le cas Aibo, il va demander une fréquence correspondant
  à la fréquence de rafraichissement des sensors et il sera appelé après que
  l'OS ai rangé les valeurs neuves des senseurs dans une structure accessible au
  hub. Le hub appelle ensuite les objets sous jacents et ceux ci transfèrent les
  valeurs de senseurs à leur variable associée.
  A priori, en créant des hubs différents, on peut avoir en principe des taux de
  rafraichissement variables selons les devices, par catégories. Ce n'est pas
  encore clair comment le kernel pourrait en tirer partie puisque pour l'instant
  le modèle est encore celui d'une execution monolithique de toutes les
  commandes de l'arbre à chaque tour. Donc, ça force à faire fonctionner le tout
  à la plus petite période demandée. Intuitivement, le kernel2 sera plus à même
  de gérer mais il faut voir si le kernel1 ne peut pas faire qqch d'intéressant
  aussi, ce n'est pas sur que non.

  Pour qu'un softdevice plugin se rattache à un UObjectHub, il suffit de passer
  son nom en paramètre au constructeur de UObject.

  NB: toutes ces structures ne sont bien entendues pas intialisées en static
  dans le vide intersidéral, mais sont gérée par une structure d'intialisation
  qui fait les choses au bon moment et dans le bon ordre (les hubs en premier,
  les UObjects ensuite, par exemple...). => étendre les baseURBIStarter pour les
  hubs? Pas forcément nécessaire.
  
   */

    
  /*
  if ((type == "event") || (type == "function"))
    URBI() << "external " << type << "(" << size << ") " << name <<";";
  */
};
	
//! UGenericCallback constructor.
UGenericCallback::UGenericCallback(string type, string name, UTable &t) : 
  name(name) , storage(0)
{
  t[this->name].push_back(this);
  /*
  URBI() << "external " << type << " " << name <<";";
  */
};

UGenericCallback::~UGenericCallback()
{
};


UGenericCallback* createUCallback(string type, void (*fun) (), string funname,UTable &t)
{
  return ((UGenericCallback*) new UCallbackGlobalvoid0 (type,fun,funname,t));
}

// **************************************************************************	
//! UTimerCallbacl constructor.

UTimerCallback::UTimerCallback(ufloat period, UTimerTable &tt) : period(period)
{
  tt.push_back(this);
  lastTimeCalled = -9999999;
}

UTimerCallback::~UTimerCallback()
{
}

	
// **************************************************************************	
//  Monitoring functions

int voidfun() {/*echo("void fun call\n");*/};

//! Generic UVar monitoring without callback
void
urbi::USync(UVar &v)
{
  urbi::UNotifyChange(v,&voidfun);
}

//! UVar monitoring with callback
void 
urbi::UNotifyChange(UVar &v, int (*fun) ())
{  
  createUCallback("var",fun,v.get_name(), monitormap);
}

//! UVar monitoring with callback including a pointeur to the UVar&
void 
urbi::UNotifyChange(UVar &v, int (*fun) (UVar&))
{
  UGenericCallback* cb = createUCallback("var",fun,v.get_name(), monitormap);
  if (cb) cb->storage = (void*)(&v);
}

//! UVar monitoring with callback including a pointeur to the UVar&
void 
urbi::UNotifyAccess(UVar &v, int (*fun) (UVar&))
{
  UGenericCallback* cb = createUCallback("varaccess",fun,v.get_name(), accessmap);
  if (cb) cb->storage = (void*)(&v);
}

//! UVar monitoring with callback, based on var name: creates a hidden UVar
void 
urbi::UNotifyChange(string varname, int (*fun) ())
{  
  createUCallback("var",fun,varname, monitormap);
}

//! UVar monitoring with callback, based on var name: creates a hidden UVar 
//! and pass it as a param in the callback
void 
urbi::UNotifyChange(string varname, int (*fun) (UVar&))
{
  UVar *hidden = new UVar(varname);
  UGenericCallback* cb = createUCallback("var",fun,varname, monitormap);
  if (cb) cb->storage = (void*)(hidden);
}

//! Timer definition
void 
urbi::USetTimer(ufloat t, int (*fun) ())
{
  new UTimerCallbacknoobj(t,fun,timermap);  
}



// **************************************************************************	
//! UObject constructor.
UObject::UObject(const string &s) :
  __name(s)
{
  objecthub = 0;
  lastUObject = this;
  UString tmps(__name.c_str()); // quelle merde ces UString!!!!
  UObj* tmpobj = new UObj(&tmps);
    
  for (urbi::UStartlist::iterator retr = urbi::objectlist->begin();
       retr != urbi::objectlist->end();
       retr++)
    if ((*retr)->name == __name)
      tmpobj->internalBinder = (*retr);
 
  // default
  derived = false;
  classname = __name;
 
  UBindVar(UObject,load);
  load = 1;
}


//! UObject destructor.
UObject::~UObject()
{  
}

void 
UObject::USetUpdate(ufloat t) 
{
  period = t;
  new UTimerCallbackobj<UObject>(t, this, &UObject::update, updatemap);
}

/*
int
UObject::updateGlobal() 
{
  update();
  for (UObjectList::iterator it = members.begin();
       it != members.end();
       it++) 
    (*it)->updateGlobal();
}
*/

// **************************************************************************	
//! UObjectHub constructor.

UObjectHub::UObjectHub(const string& s) : name(s)
{
}

//! UObjectHub destructor.
UObjectHub::~UObjectHub()
{
}

void 
UObjectHub::USetUpdate(ufloat t) 
{
  period = t;
  new UTimerCallbackobj<UObjectHub>(t, this, &UObjectHub::updateGlobal, updatemap);
}

int
UObjectHub::updateGlobal() 
{
  for (UObjectList::iterator it = members.begin();
       it != members.end();
       it++) 
    (*it)->update();
  update();
}

void 
UObjectHub::addMember(UObject* obj)
{
  members.push_back(obj);
}

UObjectList*
UObjectHub::getSubClass(string subclass)
{
  UObjectList* res = new UObjectList();
  for (UObjectList::iterator it = members.begin();
       it != members.end();
       it++)
    if ((*it)->classname == subclass)
      res->push_back(*it);

  return(res);
}


//! retrieve a UObjectHub based on its name
urbi::UObjectHub* 
urbi::getUObjectHub(string name) {

  for (urbi::UStartlistHub::iterator retr = urbi::objecthublist->begin();
       retr != urbi::objecthublist->end();
       retr++)
    if ((*retr)->name == name)
      return (*retr)->getUObjectHub();       
  
  return 0;
}
 
//! retrieve a UObject based on its name
urbi::UObject* 
urbi::getUObject(string name) {

  for (urbi::UStartlist::iterator retr = urbi::objectlist->begin();
       retr != urbi::objectlist->end();
       retr++)
    if ((*retr)->name == name)
      return (*retr)->getUObject();       
  
  return 0;
}


//! echo method
void
urbi::echo(const char* format, ... ) {

  char tmpoutput[1024];
  
  va_list arg;
  va_start(arg, format);
  vsnprintf(tmpoutput, 1024, format, arg);
  va_end(arg);

  ::urbiserver->debug(tmpoutput);
}



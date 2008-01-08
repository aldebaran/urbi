#ifndef UOBJECT_HH
# define UOBJECT_HH

#include "object/atom.hh"

//! create and return a new prototype for a bound UObject
object::rObject uobject_make_proto(std::string& name);

/*! Instanciate a new prototype inheriting from a UObject.
 A new instance of UObject is created
 \param parent parent object, created by uobject_make_proto() or uobject_new()
 \param forceName force the reported C++ name to be the class name
*/
object::rObject uobject_new(object::rObject parent, bool forceName=false);
#endif

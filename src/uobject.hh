#ifndef UOBJECT_HH
# define UOBJECT_HH

#include "object/atom.hh"

//! create and return a new prototype for a bound UObject
object::rObject uobject_make_proto(const std::string& name);

//! Instanciate a new prototype inheriting from a UObject.
object::rObject uobject_new(object::rObject proto, bool forceName=false);

//! Initialize plugin UObjects.
object::rObject uobject_initialize(runner::Runner&, object::objects_type args);
#endif

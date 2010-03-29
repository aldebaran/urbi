/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef KERNEL_UOBJECT_HH
# define KERNEL_UOBJECT_HH

# include <urbi/object/fwd.hh>

//! create and return a new prototype for a bound UObject
urbi::object::rObject uobject_make_proto(const std::string& name);

//! Instanciate a new prototype inheriting from a UObject.
urbi::object::rObject uobject_new(urbi::object::rObject proto, bool forceName=false);

//! Initialize plugin UObjects.
urbi::object::rObject uobject_initialize(const urbi::object::objects_type& args);

//! Reload uobject list
void uobjects_reload();

#endif // !KERNEL_UOBJECT_HH

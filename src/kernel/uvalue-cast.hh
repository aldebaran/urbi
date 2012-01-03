/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef KERNEL_UVALUE_CAST_HH
# define KERNEL_UVALUE_CAST_HH

# include <urbi/uvalue.hh>
# include <urbi/object/fwd.hh>

/// Cast an rObject into UValue.
urbi::UValue uvalue_cast(const urbi::object::rObject&, int recursionLevel=0);
/// Return the UValue type of an rObject
urbi::UDataType uvalue_type(const urbi::object::rObject&);
/// Cast an UValue into an rObject.
urbi::object::rObject object_cast(const urbi::UValue&);
/** Deserialize a value into urbiscript.
 *  Recurses into containers, and handles dictionaries with the magic "$sn"
 *  key as serialized classes.
 *  Urbiscript objects are instanciated by name, looking for them in lobby and
 *  Serializables.
 */
urbi::object::rObject uvalue_deserialize(urbi::object::rObject s);

#endif // !KERNEL_UVALUE_CAST_HH

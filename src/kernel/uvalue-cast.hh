/*
 * Copyright (C) 2009, Gostai S.A.S.
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
# include <object/fwd.hh>

/// Cast an rObject into UValue.
urbi::UValue uvalue_cast(const object::rObject&);
/// Return the UValue type of an rObject
urbi::UDataType uvalue_type(const object::rObject&);
/// Cast an UValue into an rObject.
object::rObject object_cast(const urbi::UValue&);

#endif // !KERNEL_UVALUE_CAST_HH

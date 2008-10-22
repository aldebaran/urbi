#ifndef KERNEL_UVALUE_CAST_HH
# define KERNEL_UVALUE_CAST_HH

# include <urbi/uvalue.hh>
# include <object/fwd.hh>

/// Cast an rObject into UValue.
urbi::UValue uvalue_cast(object::rObject);
/// Return the UValue type of an rObject
urbi::UDataType uvalue_type(object::rObject);
/// Cast an UValue into an rObject.
object::rObject object_cast(const urbi::UValue&);

#endif // !KERNEL_UVALUE_CAST_HH

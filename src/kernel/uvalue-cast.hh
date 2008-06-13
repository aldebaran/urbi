#ifndef KERNEL_UVALUE_CAST_HH
# define KERNEL_UVALUE_CAST_HH

# include <urbi/uvalue.hh>
# include <object/fwd.hh>

urbi::UValue uvalue_cast(object::rObject);
object::rObject object_cast(const urbi::UValue&);

#endif // !KERNEL_UVALUE_CAST_HH

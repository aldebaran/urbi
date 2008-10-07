/**
 ** \file object/primitive-class.hh
 ** \brief Definition of the URBI object primitive.
 */

#ifndef OBJECT_PRIMITIVE_CLASS_HH
# define OBJECT_PRIMITIVE_CLASS_HH

# include <libport/compiler.hh>
# include <object/cxx-object.hh>
# include <object/fwd.hh>

namespace object
{
  class Primitive: public CxxObject
  {
  public:
    typedef boost::function2<rObject, runner::Runner&, objects_type&> value_type;

    ATTRIBUTE_NORETURN Primitive();
    Primitive(rPrimitive model);
    Primitive(value_type value);
    value_type value_get() const;
    rObject operator() (runner::Runner& r, object::objects_type args);

    // Urbi methods
    rObject apply(runner::Runner& r, rList args);

  private:
    value_type content_;

  URBI_CXX_OBJECT(Primitive);
  };

}; // namespace object

#endif // !OBJECT_PRIMITIVE_CLASS_HH

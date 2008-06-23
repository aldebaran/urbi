/**
 ** \file object/primitive-class.hh
 ** \brief Definition of the URBI object primitive.
 */

#ifndef OBJECT_PRIMITIVE_CLASS_HH
# define OBJECT_PRIMITIVE_CLASS_HH

# include <object/cxx-object.hh>
# include <object/fwd.hh>

namespace object
{
  extern rObject primitive_class;

  class Primitive: public CxxObject
  {
  public:
    typedef boost::function2<rObject, runner::Runner&, objects_type> value_type;

    Primitive();
    Primitive(rPrimitive model);
    Primitive(value_type value);
    value_type value_get() const;

    // Urbi methods
    rObject apply(runner::Runner& r, rList args);

    static const std::string type_name;
    virtual std::string type_name_get() const;

  private:
    value_type content_;

  public:
    static void initialize(CxxObject::Binder<Primitive>& binder);
    static bool primitive_added;
  };

}; // namespace object

#endif // !OBJECT_PRIMITIVE_CLASS_HH

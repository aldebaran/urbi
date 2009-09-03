/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/**
 ** \file object/primitive-class.hh
 ** \brief Definition of the URBI object primitive.
 */

#ifndef OBJECT_PRIMITIVE_CLASS_HH
# define OBJECT_PRIMITIVE_CLASS_HH

# include <libport/compiler.hh>
# include <object/executable.hh>
# include <object/fwd.hh>

namespace object
{
  class URBI_SDK_API Primitive: public Executable
  {
  public:
    typedef boost::function1<rObject, const objects_type&> value_type;

    ATTRIBUTE_NORETURN Primitive();
    Primitive(rPrimitive model);
    Primitive(value_type value);
    value_type value_get() const;
    virtual rObject operator() (object::objects_type args);

    // Urbi methods
    rObject apply(rList args);

  private:
    value_type content_;

  URBI_CXX_OBJECT(Primitive);
  };

}; // namespace object

#endif // !OBJECT_PRIMITIVE_CLASS_HH

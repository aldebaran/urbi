/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/primitive.hh
 ** \brief Definition of the Urbi object primitive.
 */

#ifndef OBJECT_PRIMITIVE_HH
# define OBJECT_PRIMITIVE_HH

# include <libport/compiler.hh>
# include <urbi/object/executable.hh>
# include <urbi/object/fwd.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Primitive: public Executable
    {
      public:
      typedef boost::function1<rObject, const objects_type&> value_type;
      typedef std::vector<value_type> values_type;

      ATTRIBUTE_NORETURN Primitive();
      Primitive(rPrimitive model);
      Primitive(value_type value);
      values_type& value_get();
      const values_type& value_get() const;
      virtual rObject operator() (object::objects_type args);

      // Urbi methods
      rObject apply(rList args);

      private:
      values_type content_;

      URBI_CXX_OBJECT_(Primitive);
    };
  }; // namespace object
}

#endif // !OBJECT_PRIMITIVE_HH

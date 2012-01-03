/*
 * Copyright (C) 2009-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef AST_FLAVOR_HXX
# define AST_FLAVOR_HXX

# include <serialize/serialize.hh>

namespace libport
{
  namespace serialize
  {
    template <>
    struct BinaryOSerializer::Impl<ast::flavor_type>
    {
      static void put(const std::string& name, ast::flavor_type v,
                      std::ostream& output, BinaryOSerializer& ser)
      {
        Impl<unsigned>::put
          (name, static_cast<unsigned>(v), output, ser);
      }
    };

    template <>
    struct BinaryISerializer::Impl<ast::flavor_type>
    {
      static ast::flavor_type get(const std::string& name,
                             std::istream& input, BinaryISerializer& ser)
      {
        return static_cast<ast::flavor_type>
          (Impl<unsigned>::get(name, input, ser));
      }
    };
  }
}

#endif

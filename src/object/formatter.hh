/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/formatter.hh
 ** \brief Definition of the Urbi object formatter.
 */

#ifndef OBJECT_FORMATTER_HH
# define OBJECT_FORMATTER_HH

# include <libport/attributes.hh>

# include <urbi/object/fwd.hh>
# include <urbi/object/cxx-object.hh>
# include <urbi/object/cxx-primitive.hh>

namespace urbi
{
  namespace object
  {

    class Formatter: public CxxObject
    {
    public:
      Formatter();
      Formatter(rFormatter model);

      void init(const std::string& format);

      /// Regular C++ signature, all the arguments are provided.
      std::string operator%(const objects_type& args) const;

      /// Urbi signature that accepts Lists (and bounces to the function
      /// above), otherwise wraps \a arg in a List and then bounces.
      std::string operator%(const rObject& arg) const;

    private:
      /// The FormatInfos and Strings.
      ATTRIBUTE_R(rList, data);

      URBI_CXX_OBJECT(Formatter, CxxObject);
    };

  } // namespace object
}

#endif // !OBJECT_FORMATTER_HH

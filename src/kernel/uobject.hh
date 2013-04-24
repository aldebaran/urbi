/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef KERNEL_UOBJECT_HH
# define KERNEL_UOBJECT_HH

# include <serialize/binary-i-serializer.hh>
# include <urbi/object/fwd.hh>
# include <urbi/object/list.hh>
# include <urbi/details.hh>

namespace urbi
{
  namespace uobjects
  {
    /// create and return a new prototype for a bound UObject
    urbi::object::rObject
    uobject_make_proto(const std::string& name);

    /// Instanciate a new prototype inheriting from a UObject.
    urbi::object::rObject
    uobject_new(urbi::object::rObject proto,
                bool forceName=false, bool instanciate = true, const std::string& fname = "");

    /// Initialize plugin UObjects.
    ::urbi::object::rObject
    uobject_initialize(const urbi::object::objects_type& args);

    /// Find and return rObject of an UObject based on its name.
    ::urbi::object::rObject
    get_base(const std::string& objname);

    /// Process serialized request from a remote, return urbiscript to eval.
    std::string
    processSerializedMessage(int msgType,
                             libport::serialize::BinaryISerializer& v);
  }
}

/// Reload uobject list
void uobjects_reload();

/// UObject load path.
const libport::file_library uobject_uobjects_path();

#endif // !KERNEL_UOBJECT_HH

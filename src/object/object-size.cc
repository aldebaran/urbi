/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <object/finalizable.hh>
#include <object/format-info.hh>
#include <object/formatter.hh>
#include <object/profile.hh>
#include <object/semaphore.hh>
#include <object/socket.hh>
#include <object/uconnection.hh>
#include <object/urbi/input-stream.hh>
#include <object/urbi/logger.hh>
#include <object/urbi/output-stream.hh>
#include <object/urbi/process.hh>
#include <object/urbi/regexp.hh>
#include <object/uvalue.hh>
#include <object/uvar.hh>
#include <urbi/object/barrier.hh>
#include <object/code.hh>
#include <urbi/object/date.hh>
#include <urbi/object/dictionary.hh>
#include <urbi/object/directory.hh>
#include <urbi/object/duration.hh>
#include <urbi/object/event-handler.hh>
#include <urbi/object/file.hh>
#include <urbi/object/job.hh>
#include <urbi/object/location.hh>

namespace urbi
{
  namespace object
  {
# define URBI_OBJECT_UNION_FIELD(Class) char Class ## _[sizeof(Class)]
    union ObjectSize
    {
      APPLY_ON_ALL_OBJECTS(URBI_OBJECT_UNION_FIELD)
    };
# undef URBI_OBJECT_UNION_FIELD

    const size_t Object::allocator_static_max_size = sizeof(ObjectSize);
  }
}

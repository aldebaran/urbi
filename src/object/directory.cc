/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <kernel/config.h>
#if HAVE_SYS_INOTIFY_H
# include <sys/inotify.h>
#endif

#include <libport/cassert>
#include <libport/compiler.hh>
#include <libport/dirent.h>
#include <libport/format.hh>
#include <libport/lockable.hh>
#include <libport/sys/types.h>
#include <libport/thread.hh>

#include <kernel/userver.hh>

#include <urbi/object/directory.hh>
#include <urbi/object/global.hh>
#include <urbi/object/path.hh>
#include <object/symbols.hh>

#include <urbi/runner/raise.hh>

using libport::BlockLock;

namespace urbi
{
  namespace object
  {
    /*--------------.
    | C++ methods.  |
    `--------------*/

    Directory::value_type
    Directory::value_get()
    {
      return path_;
    }


    /*---------------------.
    | urbiscript methods.  |
    `---------------------*/

#if HAVE_SYS_INOTIFY_H
    // Event polling thread
    typedef std::pair<rObject, rObject> _watch_map_elem_t;
    typedef boost::unordered_map<unsigned, _watch_map_elem_t> _watch_map_t;
    static _watch_map_t _watch_map;
    static libport::Lockable _watch_map_lock;
    static int _watch_fd;

    ATTRIBUTE_NORETURN
    static
    void poll()
    {
      while (true)
      {
        static const size_t name_size_max = 1024;
        static const size_t evt_size = sizeof(inotify_event);
        static const size_t buf_size = evt_size + name_size_max;

        char buffer[buf_size];
        const int len = read(_watch_fd, &buffer, buf_size);
        if (len < 0)
        {
          if (errno == EINTR)
            continue;
          else
            errnoabort("read failed");
        }
        int i = 0;
        while (i < len)
        {
          inotify_event& evt = reinterpret_cast<inotify_event&>(buffer[i]);
          i += evt_size + evt.len;

          rObject event;
          {
            BlockLock lock(_watch_map_lock);
            aver(libport::mhas(_watch_map, evt.wd));
            if (evt.mask & IN_CREATE)
              event = _watch_map[evt.wd].first;
            else if (evt.mask & IN_DELETE)
              event = _watch_map[evt.wd].second;
            else
              pabort("unrecognized inotify event");
          }

          {
            // FIXME: objects are refcounted and urbiserver->schedule is not
            // thread safe.
            object::objects_type args;
            args << new object::String(evt.name);
            ::kernel::urbiserver->schedule(event, SYMBOL(emit), args);
          }
        }
      }
    }
#endif

    // Construction
    Directory::Directory()
      : path_(new Path("/"))
    {
      proto_add(proto);
    }

    Directory::Directory(rDirectory model)
      : path_(model.get()->path_)
    {
      proto_add(model);
      create_events();
    }

    Directory::Directory(const std::string& value)
      : path_(new Path(value))
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    void
    Directory::create_events()
    {
#if HAVE_SYS_INOTIFY_H
      CAPTURE_GLOBAL(Event);
      on_file_created_ = Event->call("new");
      slot_set(SYMBOL(fileCreated), on_file_created_);
      on_file_deleted_ = Event->call("new");
      slot_set(SYMBOL(fileDeleted), on_file_deleted_);
#endif
    }

    void Directory::init(rPath path)
    {
      if (!path->exists())
        FRAISE("does not exist: %s", path->as_string());
      if (!path->is_dir())
        FRAISE("not a directory: %s", path->as_string());
      path_ = path;

#if HAVE_SYS_INOTIFY_H
      static bool started = false;
      if (!started)
      {
        _watch_fd = inotify_init();
        if (_watch_fd == -1)
          FRAISE("unable to start inotify: %s", libport::strerror(errno));
        libport::startThread(boost::function0<void>(poll));
      }
      started = true;
      int watch = inotify_add_watch(_watch_fd, path->as_string().c_str(),
                                    IN_CREATE | IN_DELETE);
      if (watch == -1)
        FRAISE("unable to watch directory: %s", libport::strerror(errno));
      {
        _watch_map_elem_t events(on_file_created_, on_file_deleted_);
        libport::BlockLock lock(_watch_map_lock);
        std::pair<_watch_map_t::iterator, bool> res =
          _watch_map.insert(_watch_map_t::value_type(watch, events));

        // inotify return a uniq identifier per path.  If events are
        // registered we just re-use them instead.  This is fine as long as
        // Directory are not mutable.
        if (!res.second)
        {
          // Update the directory references.
          events = res.first->second;
          on_file_created_ = events.first;
          on_file_deleted_ = events.second;
          // update the slots to make the modification visible in Urbiscript.
          slot_update(SYMBOL(fileCreated), on_file_created_, false);
          slot_update(SYMBOL(fileDeleted), on_file_deleted_, false);
        }
      }
#endif
    }

    void Directory::init(const std::string& path)
    {
      init(new Path(path));
    }

    // Conversions

    std::string Directory::as_string() const
    {
      return path_->as_string();
    }

    std::string Directory::as_printable() const
    {
      return libport::format("Directory(\"%s\")", path_->as_string());
    }

    // Stat
    template <rObject (*F) (Directory& d, const std::string& entry)>
    rList Directory::list()
    {
      List::value_type res;
      DIR* dir = opendir(path_->value_get().to_string().c_str());

      // There is already an appropriate error handling done for Path.
      // We can use it if we need it.
      if (!dir)
        path_->handle_any_error();

      while (dirent* entry = readdir(dir))
      {
        if (!entry)
  	path_->handle_any_error();
        std::string name = entry->d_name;
        if (name == "." || name == "..")
          continue;
        res << F(*this, name);
      }

      closedir(dir);
      return new List(res);
    }

    /*----------.
    | Details.  |
    `----------*/

    namespace details
    {
      rObject mk_string(Directory&, const std::string&);
      rObject mk_path(Directory&, const std::string&);

      rObject
      mk_string(Directory&, const std::string& entry)
      {
        return new String(entry);
      }

      rObject
      mk_path(Directory& d, const std::string& entry)
      {
        return new Path(d.value_get()->value_get() / entry);
      }
    }

    OVERLOAD_TYPE(init_bouncer, 1, 1,
                  Path,
                  (void (Directory::*)(rPath)) &Directory::init,
                  String,
                  (void (Directory::*)(const std::string&)) &Directory::init);

    /*----------.
    | Binding.  |
    `----------*/

    void
    Directory::initialize(CxxObject::Binder<Directory>& bind)
    {
      bind(SYMBOL(asList), &Directory::list<&details::mk_path>);
      bind(SYMBOL(asPrintable), &Directory::as_printable);
      bind(SYMBOL(asString), &Directory::as_string);
      bind(SYMBOL(content), &Directory::list<&details::mk_string>);

      rPrimitive p = new Primitive(&init_bouncer);
      proto->slot_set(SYMBOL(init), p);
    }

    URBI_CXX_OBJECT_REGISTER(Directory)
      : path_(new Path())
    {}
  }
}

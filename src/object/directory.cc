/*
 * Copyright (C) 2009, Gostai S.A.S.
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

#include <libport/dirent.h>
#include <libport/format.hh>
#include <libport/lockable.hh>
#include <libport/sys/types.h>
#include <libport/thread.hh>

#include <kernel/userver.hh>

#include <object/directory.hh>
#include <object/global.hh>
#include <object/path.hh>
#include <object/symbols.hh>

#include <runner/raise.hh>

using libport::BlockLock;

namespace object
{
  using boost::format;

  /*------------.
  | C++ methods |
  `------------*/

  Directory::value_type Directory::value_get()
  {
    return path_;
  }


  /*-------------.
  | Urbi methods |
  `-------------*/

#if HAVE_SYS_INOTIFY_H
  // Event polling thread
  static libport::hash_map<unsigned, std::pair<rObject, rObject> > _watch_map;
  static libport::Lockable _watch_map_lock;
  static int _watch_fd;

  static void poll()
  {
    _watch_fd = inotify_init();
    if (_watch_fd == -1)
      FRAISE("unable to start inotify: %s", libport::strerror(errno));

    static const size_t name_size_max = 1024;

    while (true)
    {
      static const size_t evt_size = sizeof(inotify_event);
      static const size_t buf_size = evt_size + name_size_max;

      char buffer[buf_size];
      const int len = read(_watch_fd, &buffer, buf_size);

      if (len < 0)
        errnoabort();

      int i = 0;
      while (i < len)
      {
        inotify_event& evt = reinterpret_cast<inotify_event&>(buffer[i]);
        i += evt_size + evt.len;

        rObject event;
        {
          BlockLock lock(_watch_map_lock);
          assert(libport::mhas(_watch_map, evt.wd));
          if (evt.mask & IN_CREATE)
            event = _watch_map[evt.wd].first;
          else if (evt.mask & IN_DELETE)
            event = _watch_map[evt.wd].second;
          else
            pabort("unrecognized inotify event");
        }

        {
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
    proto_add(proto ? proto : Object::proto);
  }

  void
  Directory::create_events()
  {
    CAPTURE_GLOBAL(Event);
    on_file_created_ = Event->call("new");
    slot_set(SYMBOL(fileCreated), on_file_created_);
    on_file_deleted_ = Event->call("new");
    slot_set(SYMBOL(fileDeleted), on_file_deleted_);
  }

  void Directory::init(rPath path)
  {
    if (!path->is_dir())
      RAISE(str(format("Not a directory: '%s'") % path->as_string()));
    path_ = path;

#if HAVE_SYS_INOTIFY_H
    int watch = inotify_add_watch(_watch_fd, path->as_string().c_str(),
                                  IN_CREATE | IN_DELETE);
    if (watch == -1)
      FRAISE("unable to watch directory: %s", libport::strerror(errno));
    {
      libport::BlockLock lock(_watch_map_lock);
      _watch_map[watch].first = on_file_created_;
      _watch_map[watch].second = on_file_deleted_;
    }
#endif
  }

  void Directory::init(const std::string& path)
  {
    init(new Path(path));
  }

  // Conversions

  std::string Directory::as_string()
  {
    return path_->as_string();
  }

  std::string Directory::as_printable()
  {
    return (boost::format("Directory(\"%s\")") % path_->as_string()).str();
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

  /*--------.
  | Details |
  `--------*/

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

  /*---------------.
  | Binding system |
  `---------------*/

  void
  Directory::initialize(CxxObject::Binder<Directory>& bind)
  {
    bind(SYMBOL(asPrintable), &Directory::as_printable);
    bind(SYMBOL(asString), &Directory::as_string);
    bind(SYMBOL(content), &Directory::list<&details::mk_path>);
    bind(SYMBOL(list), &Directory::list<&details::mk_string>);

    rPrimitive p = new Primitive(&init_bouncer);
    proto->slot_set(SYMBOL(init), p);

#if HAVE_SYS_INOTIFY_H
    libport::startThread(boost::function0<void>(poll));
#endif
  }

  rObject
  Directory::proto_make()
  {
#ifdef WIN32
    return new Directory("C:\\");
#else
    return new Directory("/");
#endif
  }

  URBI_CXX_OBJECT_REGISTER(Directory);
}

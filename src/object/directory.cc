/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
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
#include <libport/exception.hh>
#include <libport/format.hh>
#include <libport/lockable.hh>
#include <libport/path.hh>
#include <libport/sys/types.h>
#include <libport/thread.hh>

#include <urbi/kernel/userver.hh>

#include <urbi/object/date.hh>
#include <urbi/object/directory.hh>
#include <urbi/object/global.hh>
#include <urbi/object/path.hh>
#include <urbi/object/symbols.hh>

#include <urbi/runner/raise.hh>

#include <libport/cstdio>

namespace boostfs = boost::filesystem;

using libport::BlockLock;

namespace urbi
{
  namespace object
  {
    /*----------.
    | Helpers.  |
    `----------*/

    ATTRIBUTE_NORETURN
    static void
    raise_boost_fs_error(boostfs::filesystem_error& e)
    {
      FRAISE("%s", libport::format_boost_fs_error(e.what()));
    }

    static void
    check_directory(const rPath& path)
    {
      if (!path->is_dir())
        FRAISE("not a directory: \"%s\"", path->as_string());
    }

    static void
    check_exists(const rPath& path)
    {
      if (!path->exists())
        FRAISE("directory does not exist: \"%s\"", path->as_string());
    }

    ATTRIBUTE_NORETURN
    static void
    raise_file_exists(const rPath& path)
    {
      FRAISE("file exists: \"%s\"", path->as_string());
    }

    ATTRIBUTE_NORETURN
    static void
    raise_directory_exists(const rPath& path)
    {
      FRAISE("directory exists: \"%s\"", path->as_string());
    }

    static void
    check_nexists(const rPath& path)
    {
      if (path->exists())
      {
        if (path->is_dir())
          raise_directory_exists(path);
        else
          raise_file_exists(path);
      }
    }

    static void
    check_empty(const Directory& dir)
    {
      if (!dir.empty())
        FRAISE("directory not empty: \"%s\"", dir.as_string());
    }

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
        {
          KERNEL_BLOCK_LOCK();
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
              //FIXME: Do something with those...
              else if (evt.mask & IN_DELETE_SELF)
                ;
              else if (evt.mask & IN_MOVED_FROM)
                ;
              else if (evt.mask & IN_MOVED_TO)
                ;
              else if (evt.mask & IN_MODIFY)
                ;
              else if (evt.mask & IN_IGNORED)
                ;
              else
                pabort("unrecognized inotify event");
            }


            if (event)
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

    Directory::Directory(rPath path)
      : path_(path)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    OVERLOAD_TYPE(directory_init_bouncer, 1, 1,
                  Path,
                  (void (Directory::*)(rPath)) &Directory::init,
                  String,
                  (void (Directory::*)(const std::string&)) &Directory::init);

    URBI_CXX_OBJECT_INIT(Directory)
      : path_(new Path())
    {
#define DECLARE(Name, Cxx)                \
      bind(SYMBOL_(Name), &Directory::Cxx)

      DECLARE(asList,           list<&directory_mk_path>);
      DECLARE(asPath,           as_path);
      DECLARE(asPrintable,      as_printable);
      DECLARE(asString,         as_string);
      DECLARE(clear,            clear);
      DECLARE(current,          current);
      DECLARE(content,          list<&directory_mk_string>);
      DECLARE(create,           create);
      DECLARE(createAll,        create_all);
      DECLARE(basename,         basename);
      DECLARE(empty,            empty);
      DECLARE(exists,           exists);
      DECLARE(lastModifiedDate, last_modified_date);
      DECLARE(parent,           parent);
      DECLARE(remove,           remove);
      DECLARE(removeAll_,       remove_all);
      DECLARE(rename,           rename);

#undef DECLARE

      setSlot(SYMBOL(init),   new Primitive(&directory_init_bouncer));
    }

    void Directory::init(rPath path)
    {
      check_exists(path);
      check_directory(path);

      path_ = path;

#if HAVE_SYS_INOTIFY_H
      static bool started = false;
      if (!started)
      {
        _watch_fd = inotify_init();
        if (_watch_fd == -1)
          FRAISE("cannot start inotify: %s", libport::strerror(errno));
        libport::startThread(boost::function0<void>(poll));
        started = true;
      }

      int watch = inotify_add_watch(_watch_fd, path->as_string().c_str(),
                                    IN_CREATE | IN_DELETE);
      if (watch == -1)
        FRAISE("cannot watch directory: %s", libport::strerror(errno));
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

    void
    Directory::init(const std::string& path)
    {
      init(new Path(path));
    }

    rObject
    Directory::create(rObject, rPath path)
    {
      return create_directory(path);
    }

    rDirectory
    Directory::create_directory(rPath path)
    {
      bool created = false;
      check_nexists(path);

      try
      {
        created = boostfs::create_directory(path->as_string().c_str());
      }
      catch (boostfs::filesystem_error& e)
      {
        raise_boost_fs_error(e);
      }

      // Should not be raised as check is done before creating directory.
      if (!created)
        FRAISE("no directory was effectively created: \"%s\"",
               path->as_string());

      return instanciate_directory(path->as_string());
    }

    void
    Directory::create_all_recursive(rPath path)
    {
      if (path->exists() && !path->is_dir())
        raise_file_exists(path);
      if (!path->is_root())
      {
        rPath parent = path->parent();
        if (parent->as_string() != Path::cwd()->as_string())
          create_all_recursive(parent);
      }
      if (!path->exists())
        create_directory(path);
    }

    rObject
    Directory::create_all(rObject, rPath path)
    {
      create_all_recursive(path);
      return instanciate_directory(path);
    }

    // Modifiers
    rPath
    Directory::as_path() const
    {
      return new Path(path_);
    }

    void
    Directory::clear()
    {
      boostfs::path p = path_->as_string().c_str();
      boostfs::directory_iterator end_itr;
      for ( boostfs::directory_iterator itr( p );
            itr != end_itr;
            ++itr )
      {
        boostfs::remove_all(itr->path());
      }
    }

    bool
    Directory::empty() const
    {
      try
      {
        return boostfs::is_empty(path_->as_string().c_str());
      }
      catch (boostfs::filesystem_error& e)
      {
        raise_boost_fs_error(e);
      }
    }

    bool
    Directory::exists() const
    {
      return path_->exists();
    }

    rString
    Directory::basename() const
    {
      return new String(path_->basename());
    }

    rDate
    Directory::last_modified_date() const
    {
      return path_->last_modified_date();
    }

    rDirectory
    Directory::parent() const
    {
      return instanciate_directory(new Path(path_->parent()));
    }

    void
    Directory::remove()
    {
      check_empty(*this);
      try
      {
        boostfs::remove_all(path_->as_string());
      }
      catch (boostfs::filesystem_error& e)
      {
        raise_boost_fs_error(e);
      }
    }

    void
    Directory::remove_all()
    {
      check_exists(path_);
      boostfs::remove_all(boostfs::path(path_->as_string()));
    }

    rDirectory
    Directory::rename(const std::string& name)
    {
      check_nexists(new Path(name));
      boostfs::rename(path_->as_string().c_str(),
                      name.c_str());
      path_ = new Path(name);
      return this;
    }

    /*---------------------.
    | Global information.  |
    `---------------------*/

    rDirectory
    Directory::current()
    {
      return instanciate_directory(Path::cwd());
    }

    rDirectory
    Directory::instanciate_directory(rPath path)
    {
      rDirectory dir = new Directory;
      dir->create_events();
      dir->init(path);
      return dir;
    }

    rDirectory
    Directory::instanciate_directory(const std::string& name)
    {
      return instanciate_directory(new Path(name));
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

    /*--------------.
    | Conversions.  |
    `--------------*/

    std::string
    Directory::as_string() const
    {
      return path_->as_string();
    }

    std::string
    Directory::as_printable() const
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

    rObject
    directory_mk_string(Directory&, const std::string& entry)
    {
      return new String(entry);
    }

    rObject
    directory_mk_path(Directory& d, const std::string& entry)
    {
      return new Path(d.value_get()->value_get() / entry);
    }

    template
    rList Directory::list<directory_mk_string>();
    template
    rList Directory::list<directory_mk_path>();
  }
}

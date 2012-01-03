/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_STREAM_HH
# define OBJECT_STREAM_HH

# include <libport/cstdio>
# include <libport/sys/types.h> // mode_t

# include <object/urbi/export.hh>
# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
    class URBI_MODULE_API Stream: public CxxObject
    {

      /*-----------------------------.
      | Construction / Destruction.  |
      `-----------------------------*/

    public:
      Stream(int fd, bool own);
      Stream(rStream stream);
      ~Stream();

      void open(rFile f, int flags, mode_t mode, const char* error);
      /// Check that the stream is not closed.
      /// Raise an error if it is.
      void check() const;
      void close();

    protected:
      /// File descriptor.
      /// -1 if not opened.
      int fd_;
      /// Whether we own fd_, and therefore need to close it.
      bool own_;

      URBI_CXX_OBJECT(Stream, CxxObject);
    };
  }
}

#endif

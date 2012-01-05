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
# include <object/socket.hh>
# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
    class URBI_MODULE_API Stream: public CxxObject
    {
      URBI_CXX_OBJECT(Stream, CxxObject);

      /*-----------------------------.
      | Construction / Destruction.  |
      `-----------------------------*/

    public:
      Stream();
      Stream(int fd, bool own);
      Stream(rStream stream);
      ~Stream();

      void open(rFile f, libport::Socket::OpenMode mode, int extraFlags = 0,
                int createMode = 0);
      /// Whether the stream is disconnected.
      bool closed() const;
      /// Check that the stream is not closed.
      /// Raise an error if it is.
      void check() const;
      void close();

    protected:
      void open_(int fd, bool own);
      // (re)initialize socket to a new Socket.
      void new_socket_();
      rSocket socket_;
    };
  }
}

#endif

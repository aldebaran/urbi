/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_INPUT_STREAM_HH
# define OBJECT_INPUT_STREAM_HH

# include <libport/cstdio>

# include <object/semaphore.hh>

# include <urbi/object/event.hh>
# include <object/urbi/export.hh>
# include <object/urbi/stream.hh>

namespace urbi
{
  namespace object
  {
    class URBI_MODULE_API InputStream: public Stream
    {
      URBI_CXX_OBJECT(InputStream, Stream);

      /*-----------------------------.
      | Construction / Destruction.  |
      `-----------------------------*/

    public:
      InputStream(int fd, bool own);
      InputStream(rInputStream stream);
      ~InputStream();

    private:
      // Set up onReceive/onError on Stream::socket_.
      void init_socket_();

      /*--------------.
      | Data access.  |
      `--------------*/

      /// Get a byte.
      /// \return the next available byte, or -1 if none available.
      int get_();

      /// Fill the buffer.
      /// \return true if something was read.
      bool getBuffer_();
      std::string getSeparator_(char sep, bool incl, bool& ok);
      std::string buffer_;
      unsigned pos_;
      size_t size_;

      rObject receive_(objects_type args);
      rObject onError_(objects_type args);
      rSemaphore sem_; // For async read
      bool waiting_; // True if someone is waiting on sem_

      /*---------------.
      | Urbi methods.  |
      `---------------*/

    public:
      boost::optional<std::string> content();
      void init(rFile f);
      rObject get();
      boost::optional<std::string> getChar();
      boost::optional<std::string> getLine();

      /*----------.
      | Details.  |
      `----------*/

    private:
      rSubscription on_error_subscription_;
    };
  }
}

#endif

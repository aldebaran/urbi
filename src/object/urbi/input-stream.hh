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

# include <object/urbi/export.hh>
# include <object/urbi/stream.hh>

namespace urbi
{
  namespace object
  {
    class URBI_MODULE_API InputStream: public Stream
    {

      /*-----------------------------.
      | Construction / Destruction.  |
      `-----------------------------*/

    public:
      InputStream(int fd, bool own);
      InputStream(rInputStream stream);
      ~InputStream();

      /*--------------.
      | Data access.  |
      `--------------*/

    private:
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

      /*---------------.
      | Urbi methods.  |
      `---------------*/

    public:
      void init(rFile f);
      rObject get();
      rObject getChar();
      rObject getLine();

      /*----------.
      | Details.  |
      `----------*/

    private:
      URBI_CXX_OBJECT(InputStream, Stream);
    };
  }
}

#endif

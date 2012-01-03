/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_OUTPUT_STREAM_HH
# define OBJECT_OUTPUT_STREAM_HH

# include <object/urbi/stream.hh>

namespace urbi
{
  namespace object
  {
    class URBI_MODULE_API OutputStream: public Stream
    {

      /*-----------------------------.
      | Construction / Destruction.  |
      `-----------------------------*/

    public:
      OutputStream(int fd, bool own);
      OutputStream(rOutputStream stream);
      ~OutputStream();

      /*---------------.
      | Urbi methods.  |
      `---------------*/

    public:
      void init(rFile f);
      rOutputStream putByte(unsigned char);
      void flush();
      rOutputStream put(rObject o);

      /*----------.
      | Details.  |
      `----------*/

    private:
      URBI_CXX_OBJECT(OutputStream, Stream);
    };
  }
}

#endif

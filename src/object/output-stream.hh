/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#ifndef OBJECT_OUTPUT_STREAM_HH
# define OBJECT_OUTPUT_STREAM_HH

# include <object/cxx-object.hh>

namespace object
{
  class OutputStream: public CxxObject
  {

  /*-----------------------------.
  | Construction / Destruction.  |
  `-----------------------------*/

  public:
    OutputStream(int fd, bool own);
    OutputStream(rOutputStream stream);
    ~OutputStream();

  /*--------------.
  | Data access.  |
  `--------------*/

  private:
    void checkFD_() const;

  /*---------------.
  | Urbi methods.  |
  `---------------*/

  public:
    void init(rFile f);
    rOutputStream putByte(unsigned char);
    void flush();
    rOutputStream put(rObject o);
    void close();

  /*----------.
  | Details.  |
  `----------*/

  private:
    int fd_;
    bool own_;
    URBI_CXX_OBJECT(OutputStream);
  };
}

#endif

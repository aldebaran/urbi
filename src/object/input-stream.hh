/*
 * Copyright (C) 2009, Gostai S.A.S.
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

# include <object/cxx-object.hh>

namespace object
{
  class InputStream: public CxxObject
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
    int get_();
    bool getBuffer_();
    std::string getSeparator_(char sep, bool incl, bool& ok);
    int fd_;
    bool own_;
    unsigned char buffer_[BUFSIZ];
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
    URBI_CXX_OBJECT(InputStream);
  };
}

#endif

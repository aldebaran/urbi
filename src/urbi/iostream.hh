/*
 * Copyright (C) 2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_IOSTREAM_HH
# define URBI_IOSTREAM_HH

# include <libport/fifo.hh>
# include <libport/iostream>

# include <runner/runner.hh>
# include <urbi/export.hh>

namespace urbi
{
  /* Coroutine-aware streambuf.
   *
   * Fill me through my post_data API.  Read from me as a normal
   * iostream, I will kindly yield if I'm empty, and wake up when new
   * data is available.
   */
  class StreamBuffer: public std::streambuf
  {
  public:
    StreamBuffer();
    void post_data(const char* data, size_t size);
    void post_data(const std::string& data);
    void close();

  protected:
    virtual int underflow();

    virtual int overflow(int c);
    virtual int sync();

  private:
    void grow_(size_t requested);
    void wake_up_();
    struct Buffer
    {
      Buffer();
      char* buffer;
      size_t used;
      size_t size;
    };
    Buffer buffer1_;
    Buffer buffer2_;
    Buffer* buffer_read_;
    Buffer* buffer_write_;
    runner::rRunner frozen_;
    bool close_;
  };
}

#endif

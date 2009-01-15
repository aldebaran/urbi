/*! \file kernel/uqueue.hh
 *******************************************************************************

 File: uqueue.h\n
 Definition of the UQueue class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef KERNEL_UQUEUE_HH
# define KERNEL_UQUEUE_HH

# include <libport/fifo.hh>
# include <string>

namespace kernel
{
  class UQueue : public libport::Fifo<char, '\0'>
  {
  public:
    typedef libport::Fifo<char, '\0'> super_type;

    //! UQueue constructor.
    //
    // \param chunk_size The size of allocated chunks. The buffer will start
    //                   at that size and increment by the same amount when
    //                   needed.
    UQueue(super_type::size_type chunk_size = 1024);

    //! Pops the next command in the queue.
    /*! Scan the buffer to a terminating ',' or ';' symbol by removing
     any text between:

     - { and }
     - [ and ]
     - / * and * /
     - // and \\n
     - # and \\n
     - ( and )

     The final ',' or ';' is the last character of the popped data.

     \return the command popped or an empty string if there was an error or
	   nothing to pop.
     */
    std::string pop_command();

    private:
    /// Do not try to pop a command if less than preparse_hint bytes are
    /// available.
    size_t preparse_hint;
  };
}

#endif // !KERNEL_UQUEUE_HH

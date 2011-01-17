/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef GENERIC_GENERIC_HH
# define GENERIC_GENERIC_HH

# include <urbi/uobject.hh>

// FIXME: We would like to return void here, but in that case, VC++
// fails to compile our specialization of callbacks.  It would be nice
// to avoid this special-casing for void, but I don't know how.  So
// for the time being, don't use any void functions...
// # define USE_VOID

// USE_VOID is defined in uobject.hh, undef it to get the wanted behavior.
# undef USE_VOID

# ifdef USE_VOID
#  define IF_VOID(Then, Else) Then
# else
#  define IF_VOID(Then, Else) Else
# endif

using namespace urbi;

class generic : public UObject
{
public:
  generic (const std::string& s);
  int	init ();

  int foo (int x);
  IF_VOID(void, int) inc ();

  UReturn newval (UVar&);

private:
  UVar val;
};
#endif

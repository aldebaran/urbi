/*! \file uast.hh
 *******************************************************************************

 File: uast.h\n
 Definition of the UAst class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef UAST_HH
# define UAST_HH

# include "location.hh"

class UAst
{
public:
  typedef yy::location location;

  UAst (const location& l);

  const location& loc () const;

protected:
  location loc_;
};

inline UAst::UAst (const location& l)
  : loc_(l)
{
}

inline
const UAst::location&
UAst::loc () const
{
  return loc_;
}

#endif // ! UAST_HH

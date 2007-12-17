/// \file unamedparameters.hxx

#ifndef UNAMEDPARAMETERS_HXX
# define UNAMEDPARAMETERS_HXX

# include "unamedparameters.hh"

inline
UNamedParameters*
UNamedParameters::operator[] (size_t n)
{
  UNamedParameters* res = this;
  for (/* nothing */; n > 0; --n)
    res = res->next;
  return res;
}

#endif

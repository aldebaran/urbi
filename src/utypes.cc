#include <iostream>
#include <kernel/utypes.hh>

std::ostream&
operator<< (std::ostream& o, UErrorValue v)
{
  switch (v)
  {
#define CASE(V) case V: o << #V; break;
    CASE(USUCCESS);
    CASE(UFAIL);
#undef CASE
  }
  return o;
}

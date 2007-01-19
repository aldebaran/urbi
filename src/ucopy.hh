#ifndef UCOPY_HH
# define UCOPY_HH

// FIXME: this should probably be inline in some file, at the top
// level of the hierarchy.  I don't know which one currently.

namespace
{
  // FIXME: Should take a const arg, but does not work currently.
  template <typename T>
  inline
  T*
  ucopy (const T* t)
  {
    return t ? t->copy () : 0;
  }
}

#endif

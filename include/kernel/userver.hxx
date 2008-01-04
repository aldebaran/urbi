#include <kernel/userver.hh>

//! Accessor for period_.
inline ufloat
UServer::period_get() const
{
  return period_;
}

//! Accessor for lastTime_.
inline ufloat
UServer::lastTime()
{
  return lastTime_;
}


inline
int unic()
{
  /// Unique identifier to create new references.
  static int cnt = 10000;
  return ++cnt;
}

// Return an identifier starting with \a prefix, ending with a unique int.
inline
std::string unic (const char* prefix)
{
  std::ostringstream o;
  o << prefix << unic();
  return o.str();
}


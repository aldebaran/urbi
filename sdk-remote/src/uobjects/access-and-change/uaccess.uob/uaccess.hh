#ifndef UACCESS_HH
# define UACCESS_HH

# include <urbi/uobject.hh>

class uaccess : public urbi::UObject
{
public:
  uaccess (const std::string& s);
  int init ();

  urbi::UVar val;

  urbi::UReturn newval (urbi::UVar&);
};

#endif

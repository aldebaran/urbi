#ifndef UCHANGE_HH
# define UCHANGE_HH

# include <urbi/uobject.hh>

class uchange : public urbi::UObject
{
public:
  uchange (const std::string& s);
  int init ();

  urbi::UVar* val;

  urbi::UReturn newval (urbi::UVar&);
};

#endif

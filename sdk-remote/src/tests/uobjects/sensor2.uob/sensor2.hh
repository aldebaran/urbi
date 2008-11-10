#ifndef SENSOR2_HH
# define SENSOR2_HH

# include <urbi/uobject.hh>

using namespace urbi;

class sensor2 : public UObject
{
  public:
    sensor2 (std::string s);
    int	init ();

    UVar *val;

    UReturn newval (UVar&);
    UReturn getval (UVar&);
    void setVal (int);
};

#endif

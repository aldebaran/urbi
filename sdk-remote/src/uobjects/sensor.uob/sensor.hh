#ifndef SENSOR_HH
# define SENSOR_HH

# include <urbi/uobject.hh>

using namespace urbi;

class sensor : public UObject
{
  public:
    sensor (std::string s);
    int	init ();

    UVar val;

    UReturn newval (UVar&);
    UReturn getval (UVar&);
    void setVal (int);
};

#endif

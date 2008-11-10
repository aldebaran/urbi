# include <urbi/uobject.hh>

using namespace urbi;

class remote : public UObject
{
public:
  remote (const std::string& s);
  int	init ();
  int foo (int x);

  UVar* val;
  UVar toto;

  UReturn newval (UVar&);
};

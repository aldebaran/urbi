#include <urbi/uobject.hh>

class ultest : public urbi::UObject
{
public:
  ultest(const std::string& name)
    : urbi::UObject(name)
  {
    UBindFunction(ultest, f);
  }

  int f()
  {
    // Problem here.
    new urbi::UList();
    return 0;
  }

};

UStart(ultest);

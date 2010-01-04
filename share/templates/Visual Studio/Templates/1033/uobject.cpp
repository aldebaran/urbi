#include <urbi/uobject.hh>


class [!output UOBJECT_NAME]: public urbi::UObject
{
  public:
    [!output UOBJECT_NAME](const std::string& str);
    int init();
};


[!output UOBJECT_NAME]::[!output UOBJECT_NAME](const std::string& s)
 : urbi::UObject(s)
{
  UBindFunction([!output UOBJECT_NAME], init);
};

int [!output UOBJECT_NAME]::init()
{
  return 0;
};

UStart([!output UOBJECT_NAME]);

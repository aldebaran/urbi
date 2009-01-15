/// \file urbi/uobject.hh

#ifndef URBI_UOBJECT_HUB_HH
# define URBI_UOBJECT_HUB_HH

# include <urbi/fwd.hh>

namespace urbi
{

  //! Main UObjectHub class definition
  class URBI_SDK_API UObjectHub
  {
  public:

    UObjectHub(const std::string&);
    virtual ~UObjectHub();

    void addMember(UObject* obj);
    void delMember(UObject* obj);

    /// Set a timer that will call update() every 'period' milliseconds.
    void USetUpdate(ufloat period);
    virtual int update() {return 0;}

    UObjectList  members;
    UObjectList* getSubClass(const std::string&);
    //   UObjectList* getAllSubClass(const std::string&); //TODO

  protected:
    /// This function calls update and the subclass update.
    int updateGlobal();

    ufloat period;
    std::string name;
  };

} // end namespace urbi

#endif // ! URBI_UOBJECT_HUB_HH

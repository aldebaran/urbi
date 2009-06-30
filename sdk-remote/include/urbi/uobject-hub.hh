/// \file urbi/uobject.hh

#ifndef URBI_UOBJECT_HUB_HH
# define URBI_UOBJECT_HUB_HH

# include <urbi/fwd.hh>
# include <urbi/ucontext.hh>
namespace urbi
{

  //! Main UObjectHub class definition
  class URBI_SDK_API UObjectHub: public UContext
  {
  public:

    UObjectHub(const std::string&, impl::UContextImpl* ctx=0);
    virtual ~UObjectHub();

    void addMember(UObject* obj);
    void delMember(UObject* obj);

    /// Set a timer that will call update() every 'period' milliseconds.
    void USetUpdate(ufloat period);
    virtual int update() {return 0;}

    UObjectList  members;
    UObjectList* getSubClass(const std::string&);
    //   UObjectList* getAllSubClass(const std::string&); //TODO
    const std::string& get_name() { return name;}
  protected:
    /// This function calls update and the subclass update.
    int updateGlobal();

    ufloat period;
    std::string name;
  };

} // end namespace urbi

#endif // ! URBI_UOBJECT_HUB_HH

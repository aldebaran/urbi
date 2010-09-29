/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

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

    urbi::UObjectList  members;
    urbi::UObjectList* getSubClass(const std::string&);
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

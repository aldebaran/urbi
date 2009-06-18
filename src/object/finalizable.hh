#ifndef OBJECT_FINALIZABLE_HH
# define OBJECT_FINALIZABLE_HH

# include <object/cxx-object.hh>

namespace object
{
  class URBI_SDK_API Finalizable: public CxxObject
  {
  public:
    Finalizable();
    Finalizable(rFinalizable model);
    virtual ~Finalizable();

    void __inc();
    void __dec();
    int __get();
  private:
    URBI_CXX_OBJECT(Finalizable);
  };
}

#endif

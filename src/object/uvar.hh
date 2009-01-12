#ifndef OBJECT_STRING_UVAR_HH
# define OBJECT_STRING_UVAR_HH

# include <object/cxx-object.hh>

namespace object
{
  /** class providing access and change notification of a slot.
   *
   *  See uobject.u for the urbi part.
   */
  class UVar: public Primitive
  {
  public:
    UVar();
    UVar(libport::intrusive_ptr<UVar> model);
    rObject update_(runner::Runner&, rObject arg);
    rObject accessor(runner::Runner&);
    void loopCheck(runner::Runner&);
    rObject writeOwned(runner::Runner&, rObject newval);
  private:
    bool looping_;
    bool inChange_;
    bool inAccess_;
    URBI_CXX_OBJECT(UVar);
  };
}
#endif

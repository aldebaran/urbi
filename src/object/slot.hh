#ifndef OBJECT_SLOT_HH
# define OBJECT_SLOT_HH

# include <libport/assert.hh>
# include <libport/symbol.hh>

# include <object/fwd.hh>

namespace object
{
  class Slot
  {
  public:
    Slot();
    template <typename T>
    Slot(const T& value);
    template <typename T>
    T get();
    template <typename T>
    void set(const T& value);
    template <typename T>
    const T& operator=(const T& value);
    operator rObject ();
    Object* operator->();
    const Object* operator->() const;

  private:
    rObject where_;
    libport::Symbol name_;
    rObject value_;
  };
}

#endif

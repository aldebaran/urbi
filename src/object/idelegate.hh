#ifndef OBJECT_IDELEGATE_HH
# define OBJECT_IDELEGATE_HH

namespace object {
class IDelegate
{
  public:
  virtual rObject operator() (rLobby, objects_type) = 0;
  virtual ~IDelegate() {}
};
}

#endif

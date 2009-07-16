#ifndef FACTORY_FACTORY_HH
# define FACTORY_FACTORY_HH

# include <urbi/uobject.hh>

class Factory
{
public:
  Factory(unsigned duration);
  std::string operator()(const std::list<std::string>& components) const;
  unsigned duration;
};

#endif // ! FACTORY_FACTORY_HH

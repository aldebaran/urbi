#include "tag-info.hh"

TagInfo::TagInfo()
  :frozen(false), blocked(false), parent(0)
{
}

TagInfo*
TagInfo::insert(HMtagtab &tab)
{
  HMtagtab::iterator i = tab.insert(HMtagtab::value_type(name, *this)).first;
  TagInfo * result = &i->second;

  //remove last part of tag
  size_t pos = name.find_last_of('.');
  if (pos == std::string::npos) //we reached base tag
    return result;

  std::string subtag = name.substr(0, pos);
  HMtagtab::iterator it = tab.find(subtag);
  TagInfo *parent;
  if (it == tab.end())
  {
    TagInfo t;
    t.blocked = t.frozen = false;
    t.name = subtag;
    parent = t.insert(urbiserver->tagtab);
  }
  else
    parent = &it->second;

  parent->subTags.push_back(result);
  result->parent = parent;
  result->parentPtr = --parent->subTags.end();

  return result;
}

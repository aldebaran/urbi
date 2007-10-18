#include "kernel/tag-info.hh"
#include "kernel/userver.hh"

TagInfo::TagInfo()
  : frozen(false), blocked(false),
    commands(), subTags(), parent(0),
    parentPtr(), name ()
{
}

// C++ forbids the reading of uninitialized iterators (called
// "singular iterators", even when it seem benign such as when copying
// one into another (see
// http://lists.boost.org/Archives/boost/2004/09/73359.php).  Since we
// have an default-constructed iterator here (parentPtr), as a result
// we cannot use the default constructor to copy a TagInfo (which is
// the case for instance in the first instruction of TagInfo::insert
// which passes *this by copy).
//
// If we get rid of parentPtr, we can get rid of this copy ctor.
TagInfo::TagInfo(const TagInfo& rhs)
  : frozen(rhs.frozen), blocked(rhs.blocked),
    commands(rhs.commands), subTags(rhs.subTags), parent(rhs.parent),
    parentPtr(), name (rhs.name)
{
  if (rhs.parent && rhs.parentPtr != rhs.parent->subTags.end ())
    parentPtr = rhs.parentPtr;
}

TagInfo*
TagInfo::insert(HMtagtab& tab)
{
  HMtagtab::iterator i = tab.insert(HMtagtab::value_type(name, *this)).first;
  assert (i != tab.end());
  TagInfo* res = &i->second;

  // Remove last part of tag.
  size_t pos = name.find_last_of('.');
  if (pos == std::string::npos) //We reached base tag
    return res;

  // This tag has a parent.  Find it.
  std::string subtag = name.substr(0, pos);
  HMtagtab::iterator it = tab.find(subtag);
  TagInfo *parent;
  if (it != tab.end())
    parent = &it->second;
  else
  {
    // The parent did not exist yet.  Register it.
    TagInfo t;
    t.blocked = t.frozen = false;
    t.name = subtag;
    parent = t.insert(urbiserver->tagtab);
  }

  // Register ourselves in the parent.
  res->parent = parent;
  res->parentPtr = parent->subTags.insert (parent->subTags.end(), res);
  return res;
}

void
TagInfo::initializeTagInfos()
{
  // empty name, no parent, not a pb
  TagInfo* dummy = new TagInfo();

  TagInfo t;
  t.name = "__system__";
  systemTagInfo = t.insert(::urbiserver->tagtab);
  // insert a dummy tag in subtag list, so that the taginfo is never deleted
  systemTagInfo->subTags.push_back(dummy);
  t.name = "notag";
  notagTagInfo =  t.insert(::urbiserver->tagtab);
  notagTagInfo->subTags.push_back(dummy);
}

/// Cache the location of notag and system taginfos
TagInfo* TagInfo::notagTagInfo = 0;
TagInfo* TagInfo::systemTagInfo = 0;

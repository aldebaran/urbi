#ifndef TAG_INFO_HH
# define TAG_INFO_HH

/// \file tag-info.hh

# include <list>
# include <string>

# include "fwd.hh"
# include "utypes.hh" // HMtagtab

/** Structure containing informations related to a tag.
    We have a hash table of those.
    An entry survives as long as a command has the tag, or if either frozen or
    blocked is set.
    Each entry is linked to parent entry (a.b ->a) and to all commands
    having the tag.
*/

class TagInfo
{
public:
  TagInfo();

  bool frozen;
  bool blocked;
  /// All commands with this tag.
  std::list<UCommand*> commands;

  /// All tags with this one as direct parent.
  std::list<TagInfo *> subTags;
  TagInfo * parent;

  /// iterator in parent child list.
  std::list<TagInfo*>::iterator parentPtr;
  std::string name;

  /// Insert a Taginfo in map,link to parent creating if needed, recursively
  TagInfo* insert(HMtagtab& tab);
};

#endif // !TAG_INFO_HH

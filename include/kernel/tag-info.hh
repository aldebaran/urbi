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

  /// Required because of parentPtr.
  TagInfo(const TagInfo& rhs);

  /// Insert a Taginfo in \a tab, link to parent creating if needed, recursively
  TagInfo* insert(HMtagtab& tab);

  bool frozen;
  bool blocked;

  /// All commands with this tag.
  std::list<UCommand*> commands;

  /// All tags with this one as direct parent.
  std::list<TagInfo*> subTags;
  TagInfo* parent;

  /// Iterator on ourselves in parent child list.
  /// FIXME: Is this really needed?
  std::list<TagInfo*>::iterator parentPtr;

  /// The name of this tag.
  std::string name;

  /// initialize the cached taginfos.
  static void initializeTagInfos();

  /// Cached often used taginfo
  static TagInfo* systemTagInfo;
  /// Cached often used taginfo
  static TagInfo* notagTagInfo;
};

#endif // !TAG_INFO_HH

/// \file flavorable.hh

#ifndef FLAVORABLE_HH
# define FLAVORABLE_HH

# include "libport/assert.hh"

/// Root for command that come in different flavors.
class Flavorable
{
public:
  /// Node type for a UCommand_TREE
  enum UNodeType
  {
    UAND,
    UPIPE,
    USEMICOLON,
    UCOMMA
  };

  /// Initialize the type.
  Flavorable (UNodeType t)
    : flavor_ (t)
  {}

  /// Get its type.
  UNodeType flavor () const
  {
    return flavor_;
  }

  /// Flavor as string.
  const char* flavor_string () const
  {
    switch (flavor_)
    {
      case UAND:
	return "AND";
      case UPIPE:
	return "PIPE";
      case USEMICOLON:
	return "SEMICOLON";
      case UCOMMA:
	return "COMMA";
      default:
	pabort ("unexpected flavor: " << flavor_);
    }
  }

private:
  UNodeType flavor_;
};

#endif // ! FLAVORABLE_HH

#! /usr/bin/perl -w

use strict;

my %char =
(
    'AMPERSAND' => '&',
    'CARET' => '^',
    'EQ' => '=',
    'GT' => '>',
    'LT' => '<',
    'MINUS' => '-',
    'PERCENT' => '%',
    'PIPE' => '|',
    'PLUS' => '+',
    'SLASH' => '/',
    'STAR' => '*',
    'TILDA' => '~',
);

# Convert a symbol name into its representation.
sub symbol ($)
{
    my ($res) = @_;
    while (my ($code, $char) = each %char)
    {
	$res =~ s/_?$code/$char/g;
    }
    $res;
}

# Get the list of all the SYMBOL() uses.
my $symbols = `git grep -E '(DECLARE|SYMBOL) *\\('`;

my %symbol =
    map { $_ => symbol($_) }
	($symbols =~ /\b(?:DECLARE|SYMBOL) *\(([^,\)]+)/gm);

print <<'EOF';
/**
 ** \file object/symbols.hh
 ** \brief Frequently used symbol names.
 */

#ifndef OBJECT_SYMBOLS_HH
# define OBJECT_SYMBOLS_HH

# include <libport/symbol.hh>

/* Symbols are internalized, that is to say, we keep a single
   representative, and use only it to denote all the equal symbols
   (the "FlyWeight" design pattern).  It is there quite useful to
   predeclare the symbols we use, so that at runtime we don't have to
   recompute the single representative.

   Therefore, declare here all the symbols we use somewhere in the C++
   code.  */

# define SYMBOL(Sym) object::symbol_ ## Sym

# define SYMBOLS_APPLY(Macro)			   \
EOF

for (sort keys %symbol)
{
    printf "  %-48s\\\n", "Macro($_, \"$symbol{$_}\");";
}

print <<'EOF';
  /* Backslash terminator. */

namespace object
{

# define SYMBOL_DECLARE(Name, Value)		\
  extern libport::Symbol symbol_ ## Name

  SYMBOLS_APPLY(SYMBOL_DECLARE);

# undef SYMBOL_DECLARE

} // namespace object

#endif // !OBJECT_SYMBOLS_HH
EOF

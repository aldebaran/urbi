#! /usr/bin/perl -w

use strict;

my %char =
(
    'AMPERSAND' => '&',
    'BANG' => '!',
    'CARET' => '^',
    'COLON' => ':',
    'EQ' => '=',
    'GT' => '>',
    'LT' => '<',
    'MINUS' => '-',
    'PERCENT' => '%',
    'PIPE' => '|',
    'PLUS' => '+',
    'SLASH' => '/',
    'SP' => ' ',
    'STAR' => '*',
    'TILDA' => '~',
    'UL' => '_',
);

# Convert a symbol name into its representation.
sub symbol ($)
{
    my ($res) = @_;
    while (my ($code, $char) = each %char)
    {
	$res =~ s/_?${code}_?/$char/g;
    }
    $res;
}

# Check that no symbol::Symbol are called directly with literals.
my $literals = `git grep -E -n 'libport::Symbol *\\("[^"]*")'`;
die "use SYMBOL instead of direct calls to libport::Symbol:\n$literals\n"
    if $literals;

# Get the list of all the SYMBOL() uses.
#
# SYMBOL(EQ) is used when we want to denote it explicitly.
#
# DECLARE(EQ, ...) is used in the *-class.cc to bind C++ functions
# into the Urbi world.
#
# RETURN_OP(EQ) is used in the scanner to return tokens which
# semantical value is the string itself.
my $symbols = `git grep -E '(DECLARE|SYMBOL|RETURN_OP) *\\('`;

# If git failed, do not proceed.
die "git grep failed"
    unless $symbols;

my %symbol =
    map { $_ => symbol($_) }
	($symbols =~ /\b(?:BOUNCE|DECLARE|RETURN_OP|SYMBOL) *\(([^,\)]*)/gm);

print <<'EOF';
/**
 ** \file object/precompiled-symbols.hh
 ** \brief Predefined symbols.
 */

#ifndef OBJECT_PRECOMPILED_SYMBOLS_HH
# define OBJECT_PRECOMPILED_SYMBOLS_HH

# define SYMBOLS_APPLY(Macro)			  \
EOF

for (sort keys %symbol)
{
    printf "  %-48s\\\n", "Macro($_, \"$symbol{$_}\");";
}

print <<'EOF';
  /* Backslash terminator. */

#endif // !OBJECT_PRECOMPILED_SYMBOLS_HH
EOF

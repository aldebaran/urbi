#! /usr/bin/perl -w

use strict;

my %char =
(
    'AMPERSAND' => '&',
    'BANG'      => '!',
    'CARET'     => '^',
    'COLON'     => ':',
    'DOLLAR'    => '$',
    'EQ'        => '=',
    'GT'        => '>',
    'LPAREN'    => '(',
    'LT'        => '<',
    'MINUS'     => '-',
    'PERCENT'   => '%',
    'PIPE'      => '|',
    'PLUS'      => '+',
    'RPAREN'    => ')',
    'SBL'       => '[',
    'SBR'       => ']',
    'SLASH'     => '/',
    'SP'        => ' ',
    'STAR'      => '*',
    'TILDA'     => '~',
    'UL'        => '_',
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
my $literals = `grep -E -n 'libport::Symbol *\\("[^"]*"\\)' @ARGV`;
die "use SYMBOL instead of direct calls to libport::Symbol:\n$literals\n"
    if $literals;

# Get the list of all the SYMBOL() uses.
#
# SYMBOL(EQ) is used when we want to denote it explicitly.
#
# CAPTURE_GLOBAL(EQ) is used when we want to cache a global
# symbol from C++ the first time we look it up.
#
# DECLARE(EQ, ...) is used in object/*.cc to bind C++ functions
# into the Urbi world.  Similarly with BOUNCE.
#
# RETURN_OP(EQ) is used in the scanner to return tokens which
# semantical value is the string itself.
my $symbol_tag = 'BOUNCE|CAPTURE_GLOBAL|DECLARE|SYMBOL|RETURN_OP';

# The lines that declare a symbol.
my $symbols = `grep -E '($symbol_tag) *\\(' @ARGV`;

die "grep failed"
    unless $symbols;

# The set of symbols used.
# Accept only identifiers followed by ')', or ','.
my %symbol =
    map { $_ => symbol($_) }
	($symbols =~ /\b(?:$symbol_tag) *\((\w+)\s*[,)]/gm);

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

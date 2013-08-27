package BuildAux::Utils;

=head1 NAME

BuildAux::Utils - same utility functions

=head1 SYNOPSIS

  use BuildAux::Utils;

=head1 DESCRIPTION

This perl module provides various general purpose functions.

=cut

use strict;
use BuildAux::Verbose;
use Exporter;

our @ISA = qw (Exporter);
our @EXPORT = qw ($balanced $braced
                  &file_comment_type &file_type
                  &license
                  &make_comment
                  &shellescape &strip_spaces
                  &unique
                  &xsystem);

=item C<$balanced>

A regexp that matches 0 or more repetitions of either a
non-backtracking word with no parens/braces, or a braced group, or a
parenthezied group.

Note that since it is greedy and non-backtracking, it might eat too
much.  Use C<$braced> instead.

=cut

# Declared and built in two steps, as it is recursive.
our $balanced;
$balanced =
  qr{
       (?:
          (?> [^(){}\[\]]+ )         # Non-parens/braces without backtracking
        |
          \( (??{ $balanced }) \)    # Group with matching parens
        |
          \{ (??{ $balanced }) \}    # Group with matching braces
        |
          \[ (??{ $balanced }) \]    # Group with matching brackets
       )*
    }x;

=item C<$braced>

Same as above, but allow backtracking on non-braced parts, so that
we don't eat places where we want to catch operators.

=cut

our $braced =
  qr{
      (?:
          \{ $balanced \}
        | \( $balanced \)
        | \[ $balanced \]
        | [^(){}\[\]]+        # Non-parens/braces with backtracking
      )*
    }x;

=item C<file_comment_type ($file)>

The type of comment to use for C<$file>.  Return C<c>, C<c++>
(including for C<java> and C<urbiscript>), C<sh> (including for
C<makefile> and <m4>) or C<tex>.

=cut

sub file_comment_type ($)
{
  my ($file) = @_;
  # The comment type to use.
  my $res = file_type($file);
  if ($res)
  {
    $res = 'c++'
      if $res eq 'java' || $res eq 'urbiscript';
    $res = 'sh'
      if $res eq 'makefile' || $res eq 'm4';
  }
  $res;
}


=item C<file_type ($file)>

The type of C<$file>, based on its name.  Return values: C<c>, C<c++>,
C<m4>, C<makefile>, C<sh>, C<tex>, C<urbiscript>.

=cut

sub file_type ($)
{
  my ($file) = @_;
  $file =~ s/\.in\z//;
  (my $ext = $file) =~ s{.*\.}{};
  my $res;
  if ($ext =~ m{\A(?:h|c)\z})
  {
    $res = 'c';
  }
  elsif ($ext =~ m{\A(?:cc|cpp|hh|hpp|hxx)\z})
  {
    $res = 'c++';
  }
  elsif ($ext =~ m{\A(?:java)\z})
  {
    $res = 'java';
  }
  elsif ($ext =~ m{\A(?:ac|m4|m4sh)\z})
  {
    $res = 'm4';
  }
  elsif ($ext =~ m{\A(?:am|mk)\z})
  {
    $res = 'makefile';
  }
  elsif ($ext eq 'sh')
  {
    $res = 'sh';
  }
  elsif ($ext eq 'tex')
  {
    $res = 'tex';
  }
  elsif ($ext eq 'u')
  {
    $res = 'urbiscript';
  }
  $res;
}


=item C<make_box($prologue, $prefix, $epilogue, $body)>

Return C<$body> with each line prefixed by C<$prefix>, and with
C<$prologue> before and C<$epilogue> after, both on single lines.

=cut

sub make_box($$$$)
{
  my ($prologue, $prefix, $epilogue, $body) = @_;
  my $res = $body;
  $res =~ s/^/$prefix/gm;
  $res = $prologue . "\n" . $res
    if $prologue;
  $res .= $epilogue . "\n"
    if $epilogue;

  # Strip trailing white spaces.
  $res =~ s/[\t ]+$//mg;

  $res;
}


=item C<make_comment($language, $body, $doc)>

Return C<$body> turned into a comment for C<$language>.  C<$language>
can be C<c> (C</*...*/), C<c++> (C<// ...>), or C<tex> (C<%% ...>).

If C<$doc> is given, and is true, then issue documentation comments.

=cut

sub make_comment($$;$)
{
  my ($language, $body, $doc) = @_;
  my %markup =
    (
     c     => ['/*', ' * ', ' */'],
     'c++' => ['',   '// ', ''],
     m4    => ['',   '## ', ''],
     sh    => ['',   '## ', ''],
     tex   => ['',   '%% ', ''],
    );
  if (defined $doc && $doc)
  {
    $markup{'c'  } = ['/**', ' * ', ' */'];
    $markup{'c++'} = ['',   '/// ', ''];
  }
  my ($pre, $in, $post) = @{$markup{$language}};
  make_box($pre, $in, $post, $body);
}


=item C<license($language, [$years = current-year])>

The Gostai license file header for C<$years>, as a comment for
C<$language>.  See C<make_comment> for the available languages.

=cut

sub license($;$)
{
  my ($language, $years) = @_;
  $years = 1900 + @{[localtime]}[5]
    unless defined $years;

  my $res = make_comment($language, <<EOF);
Copyright (C) $years, Aldebaran Robotics

This software is provided "as is" without warranty of any kind,
either expressed or implied, including but not limited to the
implied warranties of fitness for a particular purpose.

See the LICENSE file for more information.
EOF
}


=item C<shellescape($str)>

Return C<$str> properly escaped for a regular Bourne shell.  Avoid
useless quotes to produce a readable result.

This is insufficient.  We actually need to have the result "eval"'ed.

=cut

sub shellescape($)
{
  my ($str) = @_;
  if ($str =~ m([^-=+_/.[:alnum:]]))
    {
      $str =~ s,[\\'],'\\$&',g;
      $str = "'$str'";
    }
  return $str;
}


=item C<strip_spaces($string)>

Return C<$string> stripped from leading/trailing spaces.

=cut

sub strip_spaces ($)
{
  local ($_) = @_;
  s/^\s+//;
  s/\s+$//g;
  $_;
}

=item C<unique(@list)>

Return C<@list> removing duplicates.

=cut

sub unique (@)
{
  my (@list) = @_;
  my %seen;
  my @res;
  for my $item (@list)
  {
    if (! exists $seen{$item})
    {
      $seen{$item} = 1;
      push @res, $item;
    }
  }
  @res;
}

=item C<xsystem(@argv)>

Run C<system(@argv)>.  Fail on failure.

=cut

sub xsystem (@)
{
  my (@args) = @_;

  verbose 2, "running: @args";
  system(@args) == 0
    or die "$me: system @args failed: $?\n";
  if ($? == -1)
  {
    die "$me: failed to execute: $!\n";
  }
  elsif ($? & 127)
  {
    die sprintf ("$me: child died with signal %d, %s coredump\n",
                 ($? & 127),  ($? & 128) ? 'with' : 'without');
  }
  elsif ($? >> 8)
  {
    die sprintf "$me: child exited with value %d\n", $? >> 8;
  }
}

1; # for require

### Setup "Gostai" style for perl-mode and cperl-mode.
## Local Variables:
## perl-indent-level: 2
## perl-continued-statement-offset: 2
## perl-continued-brace-offset: -2
## perl-brace-offset: 0
## perl-brace-imaginary-offset: 0
## perl-label-offset: -2
## cperl-indent-level: 2
## cperl-brace-offset: 0
## cperl-continued-brace-offset: -2
## cperl-label-offset: -2
## cperl-extra-newline-before-brace: t
## cperl-merge-trailing-else: nil
## cperl-continued-statement-offset: 2
## End:

package BuildAux::Verbose;

=head1 NAME

BuildAux::Verbose - log message

=head1 SYNOPSIS

  use BuildAux::Verbose

=head1 DESCRIPTION

This perl module provides the C<verbose> function and its
infrastructure.

=cut

use strict;
use File::Basename;
use Exporter;

our @ISA = qw (Exporter);
our @EXPORT = qw ($me
                  &stderr
                  &verbose $verbose);

# The prefix for the log messages.
our $me = basename($0);

# Verbosity level.
our $verbose = 1;

=item C<stderr(@message)>

Report the C<@message>.

=cut

sub stderr(@)
{
  my (@message) = @_;
  map { print STDERR "$me: $_\n" } @message;
}

=item C<verbose($level, @message)>

Report the C<@message> is C<$level> E<lt>= C<$verbose>.

=cut

sub verbose($@)
{
  my ($level, @message) = @_;
  stderr (map { "  " x $level . "$_" } @message)
    if $level <= $verbose;
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

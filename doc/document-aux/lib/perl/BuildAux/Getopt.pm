package BuildAux::Getopt;

=head1 NAME

BuildAux::Getopt - handling command line arguments

=head1 SYNOPSIS

  use BuildAux::Getopt

=head1 DESCRIPTION

This perl module factors the handling of command line arguments,
including options such as C<--help> and C<--verbose>.

=cut

use strict;
use BuildAux::Verbose;

use Exporter;

our @ISA = qw (Exporter);
our @EXPORT = qw (&getopt
                  $message);

our $message;

=item C<help($verbose)>

Generate the B<--help> message, or the full man page if C<$verbose>.

=cut

sub help ($)
{
  my ($verbose) = @_;
  use Pod::Usage;
  # See <http://perldoc.perl.org/pod2man.html#NOTES>.
  pod2usage(
    {
      -message => $message,
      -exitval => 0,
      -verbose => $verbose,
      -output  => \*STDOUT
    });
}

=item C<getopt(%option)>

Check the common lines arguments and execute the corresponding code.
Add support for common options: C<-h>, C<--help> ; C<-q>, C<--quiet> ;
C<-v>, C<--verbose>.

=cut

sub getopt (%)
{
  my %option = (@_);
  use Getopt::Long;
  Getopt::Long::Configure ("bundling", "pass_through");
  GetOptions
    (
     "h|help"    => sub { help ($verbose) },
     "q|quiet"   => sub { --$verbose },
     "v|verbose" => sub { ++$verbose },
     %option,
    )
    or exit 1;
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

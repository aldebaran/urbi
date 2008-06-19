#! /usr/bin/perl -w
# $Id$

use strict;
use File::Basename;
use Getopt::Long;

=head1 Variables
=cut

my $me = basename ($0);
my $bib;
my $tmp = "/tmp/$me";
my @project =
  (
   "APMC",
   "Image",
   "Olena",
   "Software",
   "Tiger",
   "Transformers",
   "Urbi",
   "Vaucanson",
   );

# Should submitted articles be displayed?
# By default, of course not, this is not public information.
# Set it to true for the direction, CTI etc.
my $with_submitted = 0;
my $rejected = 'note:"[rR]ejected"';
my $submitted = 'note:"[sS]ubmitted"';
my @accepted = ("!$rejected", "!$submitted");
my @year;

# Set in the order we want to see them.
my @category =
(
 ["Livres",    ['$type="BOOK"']],
 ["Revues",    ['$type="ARTICLE"']],
 ["Conférences Internationales",
	       ['$type="INPROCEEDINGS"', '!category="national"']],
 ["Conférences Nationales",
	       ['$type="INPROCEEDINGS"', 'category="national"']],
 ["Rapports de Recherche",
	       ['$type="TECHREPORT"']]
 );

=head1 Routines

=over 4

=item C<my $ref = y2k ($ref)>
Transform C<$ref> to be y2k proof.
=cut

sub y2k ($)
{
  my ($ref) = @_;
  $ref =~ s/\.([89]\d)\./.19$1./;
  $ref =~ s/\.(\d{2})\./.20$1./;
  return $ref;
}

=item C<verbose (@message)>
Report the C<@message> on C<STDERR>.
=cut

my $verbose = 0;
sub verbose (@)
{
  print STDERR "$me: @_\n"
    if $verbose;
}


=item C<display_help>
Display help and exit
=cut

sub display_help ()
{
  print <<EOF;
usage: $0 FILE.BIB

Create a LaTeX file describing the contents of FILE.BIB.

Options:
  --help             display this help and exit
  --with-submitted   also include submitted papers
  --verbose          display tool invocations
EOF
  exit 0;
}

=item C<my @ref = bibgrep (@condition)>
Return the set of BibTeX keys that match the C<@condition>.  Return
them sorted appropriately.
=cut

sub bibgrep (@)
{
  my $cites = "$tmp.cites";
  unlink $cites
    if -f $cites;
  my $cmd = ("bib2bib $bib -q -ob /dev/null -oc $tmp.cites "
	     . join (' -c ', ('', @_)));
  $cmd =~ s/([<>|\"\\\$])/\\$1/g;
  verbose @_;
  system $cmd and die;
  my @res = ();
  if (-f $cites)
    {
      my $res = `cat $tmp.cites`;
      unlink $cites;
      @res = split ("\n", $res);
    }

  @res = sort { y2k ($a) cmp y2k ($b) } @res;
  return @res;
}


=item C<my $cite = citeitem (@key)>
Produce a list of C<\citeitem{*}> for these C<@key>.
=cut

sub citeitem (@)
{
  join ("\n", map ("  \\citeitem{$_}", @_));
}


=item C<my $text = bibliography (@key)>
Produce a bibliography for given keys.
=cut
sub bibliography (@)
{
  my $cites = citeitem (@_);
  <<EOF;
\\begin{biblist}
$cites
\\end{biblist}
EOF
}


=item C<>
=cut

# Produce a subsection for the keys.
sub sectionning ($$@)
{
  my ($sectionning, $title, @key) = @_;
  my $res = "";

  $res = ("\\$sectionning\{$title\}\n"
	  . bibliography (@key)
	  . "\n")
    if (@key);

  $res;
}

=item C<my $subsection = subsection ($title, @key)>
Produce a subsection for the C<@key>.
=cut
sub subsection ($@)
{
  my ($title, @key) = @_;
  return sectionning ("subsection", $title, @key);
}


=item C<my $subsubsection = subsubsection ($title, @key)>
Produce a subsubsection for the C<@key>.
=cut
sub subsubsection ($@)
{
  my ($title, @key) = @_;
  return sectionning ("subsubsection", $title, @key);
}


=item C<my $subsubsection = year_by_publication_type ($year)>
Produce the list of the publication of this year according
to the type of the publication: conference, revue etc.  If C<$year> is not
composed of digits, it is understood as "submissions".
=cut
sub year_by_publication_type ($)
{
  my ($year) = @_;
  my $res;
  my @criteria;
  if ($year !~ /^\d+$/)
    {
      return
	unless $with_submitted;
      @criteria = ($submitted);
      $res .= "\\subsection{Soumissions en cours}\n";
    }
  else
    {
      @criteria = ("year=$year", "!$rejected", "!$submitted");
      $res .= "\\subsection{Année $year}\n";
    }
  $res .= "\\label{publications.$year}\n";
  $res .= "\n";

  for my $cat (@category)
    {
      $res .= subsubsection ($$cat[0],
			     bibgrep (@criteria, @{$$cat[1]}));
    }

  $res .= "\n";
  $res .= "\n";
  return $res;
}

=item C<my $subsubsection = year_by_publication_type ($year)>
Produce the list of the publication of this year according
to the type of the publication: conference, revue etc.  If C<$year> is not
composed of digits, it is understood as "submissions".
=cut
sub count_by_publication_type_by_year ($)
{
  my ($year) = @_;
  my @res;
  my @criteria;
  if ($year !~ /^\d+$/)
    {
      die "incorrect year: $year"
	unless $with_submitted;
      push @res, "Soumissions";
      @criteria = ($submitted);
    }
  else
    {
      push @res, $year;
      @criteria = ("year=$year", "!$rejected", "!$submitted");
    }

  for my $cat (@category)
    {
      push @res, scalar (bibgrep (@criteria, @{$$cat[1]}));
    }
  return @res;
}


=item C<my $section = section_by_publication_type ()>
Issue a section with publications classified by type.
=cut
sub section_by_publication_type ()
{
  '\section{Publications classées par catégories de publication}'
    . "\n"
    . join ("\n\n",
	    map (year_by_publication_type ($_), @year));
}


=item C<my $section = count_by_publication_type ()>
=cut
sub count_by_publication_type ()
{
  # The total foreach kind of publication and each year.
  my @total = ("Total");
  foreach my $year (@year)
    {
      my @count = count_by_publication_type_by_year ($year);
      for my $i (1..$#count)
	{
	  $total[$i] += $count[$i];
	}
    }

  # The count for each year.
  my @count =
    map (join (' & ', count_by_publication_type_by_year ($_)),
	 @year);

  '\begin{center}'
  . "\n"
  . '\begin{tabular}{|*{6}{r|}}'
  . "\n"
  . '\hline'
  . "\n"
  . ' & &  & \textbf{Conférence} & \textbf{Conférence} & \textbf{Rapport de}\\\\'
  . "\n"
  . ' \textbf{Année} & \textbf{Livre} & \textbf{Journal} & \textbf{internationale} & \textbf{nationale} & \textbf{recherche}\\\\'
  . "\n"
  . '\hline'
  . "\n"
  . join ("\\\\\n", @count, '')
  . '\hline'
  . "\n"
  . join (' & ', @total)
  . "\\\\\n"
  . '\hline'
  . "\n"
  . '\end{tabular}'
  . "\n"
  . '\end{center}'
  . "\n";
}



=item C<my $subsection = subsection_by_project ($project)>
Return a subsection for a given C<$project>.
=cut

sub subsection_by_project ($)
{
  my ($project) = @_;
  my @criteria = ("Project:\"$project\"",  "!$rejected");
  return subsection ("Projet $project", bibgrep (@criteria));
}


=item C<my $section = section_by_project ()>
Return a section covering all the projects.
=cut

sub section_by_project ()
{
  '\section{Publications classées par projet}'
    . "\n"
    . join ("\n\n",
	    map (subsection_by_project ($_), @project));
}


=item C<my $section = section_by_key ()>
List all the publications, sorted by keys, and with keys displayed.
=cut

sub section_by_key ()
{
  return sectionning ("section", "Publications classées par clé",
		      bibgrep ("!$rejected"));
}


## -------------------- ##
## Produce the output.  ##
## -------------------- ##
sub output ()
{
  print <<'EOF';
\documentclass{article}
  \usepackage[latin1]{inputenc}
  \usepackage{a4wide}
  \usepackage{bibentry,natbib}
  \usepackage[colorlinks,linkcolor=blue,urlcolor=blue]{hyperref}
  \usepackage[francais]{babel}
  \usepackage[biblio]{lrde-bst}
  \usepackage{misc} %% to get the definition of Curve.

\bibliographystyle{lrde}
%% This command is output by lrde.bst, based on apalike.bst,
%% with special commands for urllrde BibTeX field.
%% It was done with this macro, because when I (AD) tried to
%% output the url directly in the bbl, a 'protect' appeared in the
%% output, and I could not get rid of it.
\newcommand{\urllrde}[1]{\url{http://publis.lrde.epita.fr/#1}}

\title{Bibliographie du LRDE}
EOF

  if ($with_submitted)
  {
    print <<'EOF';
\newenvironment{biblist}{\begin{description}}{\end{description}}
\newcommand{\citeitem}[1]{\item[#1] \bibentry{#1}.}
EOF
  }
  else
  {
    print <<'EOF';
\newenvironment{biblist}{\begin{enumerate}}{\end{enumerate}}
\newcommand{\citeitem}[1]{\item \bibentry{#1}.}
EOF
  }

my $svn_revision = `LANG=C svn info | sed -n 's/^Revision: \\(.*\\)/\\1/p'`;
chomp $svn_revision;
print "\\author{Rev. $svn_revision}\n";

(my $bibname = $bib) =~ s/.*\///;

print <<EOF;
\\date{\\today}

\\begin{document}
\\nobibliography{$bibname}
\\maketitle

Ce document contient la liste des articles (acceptés et soumis) à
l'écriture desquels les membres du LRDE ont collaboré depuis 1999.

\\bigskip

Le tableau suivant résume de manière quantitative le document.
\\og{}Journal\\fg{} et \\og{}conférences\\fg{} ne font référence qu'aux
publications relues par des pairs.  Le corps de ce document est
consacré à la bibliographie détaillée classée selon différents
critères.

EOF
# Emacs:'
print count_by_publication_type ();
print <<'EOF';
\newpage
\tableofcontents
\newpage
EOF
print section_by_publication_type ();
print section_by_project ();
print section_by_key ();

print <<'EOF';
\end{document}
EOF
}


## ------ ##
## Main.  ##
## ------ ##

Getopt::Long::GetOptions
  (
   'with-submitted' => \$with_submitted,
   'help'           => \&display_help,
   'verbose'        => \$verbose,
   )
  or exit 1;
$bib = $ARGV[0];
unshift @year, reverse (1999 .. 2008);
unshift @year, "soumission"
  if $with_submitted;
output ();


### Setup "GNU" style for perl-mode and cperl-mode.
## Local Variables:
## perl-indent-level: 2
## perl-continued-statement-offset: 2
## perl-continued-brace-offset: 0
## perl-brace-offset: 0
## perl-brace-imaginary-offset: 0
## perl-label-offset: -2
## cperl-indent-level: 2
## cperl-brace-offset: 0
## cperl-continued-brace-offset: 0
## cperl-label-offset: -2
## cperl-extra-newline-before-brace: t
## cperl-merge-trailing-else: nil
## cperl-continued-statement-offset: 2
## End:

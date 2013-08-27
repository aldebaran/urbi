package BuildAux::FileUtils;

=head1 NAME

BuildAux::FileUtils - handling files

=head1 SYNOPSIS

  use BuildAux::FileUtils;

=head1 DESCRIPTION

This perl module provides various general purpose file handling functions.

=cut

use strict;
use BuildAux::Verbose;
use File::Basename;
use File::stat;

use Exporter;

our @ISA = qw (Exporter);
our @EXPORT =
  qw (&contents
      &ensure_dir &ensure_parent_dir
      &file_chmod
      &file_diff
      &file_find
      &file_install
      &file_mode &file_mtime &file_mtime_set
      &file_rename &file_remove
      &file_same &file_save &file_stat &file_symlink
      &file_update &file_update_file
      );

=item C<contents ($file)>

The contents of C<$file> as a single string.

=cut

sub contents ($)
{
  my ($file) = @_;
  local $/;                     # Turn on slurp-mode.
  use BuildAux::XFile;
  my $f = new BuildAux::XFile "$file";
  my $res = $f->getline;
  $f->close;
  $res;
}

=item C<ensure_dir($dir)>

Create, if needed, the directory C<$dir>.

=cut

sub ensure_dir ($)
{
  my ($dir) = @_;
  if (! -d $dir)
    {
      use File::Path;
      mkpath ($dir, { verbose => $verbose, })
        or die "$0: cannot create $dir: $!";
    }
}

=item C<ensure_parent_dir($filename)>

Create, if needed, the directory that will contain I<$filename>.

=cut

sub ensure_parent_dir ($)
{
  my ($file) = @_;
  ensure_dir (dirname ($file));
}


=item C<file_chmod($file, $mode)>

Change the permissions of C<$file> to C<$mode>.  Fail on errors.

=cut

sub file_chmod($$)
{
  my ($file, $mode) = @_;
  chmod($mode, $file)
    or die "$me: cannot chmod $file: $!\n";
}


=item C<file_diff($old, $new)>

Display the diffs between the files C<$old> and C<$new> on stderr.
Return the exit status of diff: 0 iff the files are equal.

=cut

# Emacs fools colordiff, the output is ugly.  Likewise with buildbot.
my $diff = 'diff';

sub file_diff ($$)
{
  my ($old, $new) = @_;
  system("$diff -u $old $new >&2");
  $? >> 8;
}

=item C<file_find($file, @path)>

Look for C<$file> in C<@path> (using C<-f>).  Return C<undef> on
failure.

=cut

sub file_find($@)
{
  my ($file, @path) = @_;
  for my $d (@path)
  {
    return "$d/$file"
      if -f "$d/$file";
  }
  undef;
}


=item C<file_install($from, $to)>

Copy the file C<$from> as the file C<$to>.  Create parent directories
if needed.

Preserve permissions.  This avoids spurious warnings from ldd that
does not like that shared objects are not executable.

=cut

sub file_install($$)
{
  my ($from, $to) = @_;
  verbose 3, "copy $from $to";
  ensure_parent_dir($to);

  use File::Copy;
  File::Copy::copy $from, $to
    or die "$me: cannot copy $from $to: $!\n";

  file_chmod($to, file_mode($from));
}

=item C<file_mode($file)>

Return the permissions of C<$file>.

=cut

sub file_mode($)
{
  file_stat(@_)->mode & 0777;
}

=item C<file_mtime($file)>

Return the modification time of C<$file>.

=cut

sub file_mtime($)
{
  file_stat(@_)->mtime;
}

=item C<file_remove($file)>

Unlink C<$file>.  Fail on errors.

=cut

sub file_remove($)
{
  my ($file) = @_;
  unlink ($file)
    or die "$me: cannot remove $file: $!\n";
}

=item C<file_rename($old, $new)>

Rename file C<$old> as C<$new>.  Fail on errors.

=cut

sub file_rename($$)
{
  my ($old, $new) = @_;
  rename ($old, $new)
    or die "$me: cannot rename $old as $new: $!\n";
}

=item C<file_same($path1, $path1)>

Whether C<$path1> and C<$path2> point to the same file.  We do not
work on path names because there are too many ways to name a single
file, in particular when winepath goes into the scene.

=cut

sub file_same ($$)
{
  my ($path1, $path2) = @_;
  my ($s1) = file_stat($path1);
  my ($s2) = file_stat($path2);
  $s1->dev == $s2->dev && $s1->ino == $s2->ino;
}


=item C<file_save($filename, $contents)>

Save in C<$filename> the C<$contents>.  If C<$filename> is empty or
undefined, send to stdout.

=cut

sub file_save($$)
{
  my ($file, $contents) = @_;
  ensure_parent_dir($file);
  if ($file eq '')
  {
    print $contents;
  }
  else
  {
    my $out = new BuildAux::XFile(">$file");
    print $out $contents;
    $out->close;
  }
}

=item C<file_mtime_set($file, $mtime)>

Change the modification of C<$file>.

=cut

sub file_mtime_set($$)
{
  my ($file, $mtime) = @_;
  utime (file_stat($file)->atime, $mtime, $file)
    or die "$me: cannot utime $file: $!\n";
}

=item C<file_stat($file)>

Return the stat of C<$file>, fail on errors.

=cut

sub file_stat($)
{
  my ($file) = @_;
  my $res = stat($file)
    or die "$me: cannot stat $file: $!\n";
  $res;
}

=item C<file_symlink($old, $new)>

Create C<$new> as a symbolic link to C<$old>.  Create parent
directories if needed.  Relative addresses are treated as in C<cp>
(relative to the current working directory), not as in C<ln -s>
(relative to the destination).  If C<$old> already exists, override it.

=cut

sub file_symlink($$)
{
  my ($old, $new) = @_;
  verbose 3, "symlink $old $new";
  ensure_parent_dir($new);

  # If $new is relative, make it relative to $old instead of relative
  # to $cwd.  That's ln -s's semantics.
  my $rel = $old;
  if (!File::Spec->file_name_is_absolute($rel))
  {
    verbose 3, "Making absolute";
    $rel = File::Spec->rel2abs($rel);
  }

  verbose 3, "symlink $old (as $rel) $new";
  file_remove $new
    if -e $new;
  symlink $rel, $new
    or die "$me: cannot symlink $rel $new: $!\n";
}

=item C<file_update($filename, $new, [%option])>

Update the contents of C<$filename> to C<$new> if it is different from
the current content.  Save the file contents as C<$filename.bak>, and
display the diffs.  Preserve the permissions.

If C<$filename> is empty or undefined, send to stdout.

The supported options are:

  ${option}{mtime} preserve modification time even if the file is updated.
  ${option}{nobak} remove back up files.

=cut

sub file_update ($$%)
{
  my ($file, $new, %option) = @_;

  if (!defined $file || $file eq '')
  {
    print $new;
  }
  elsif (! -f $file)
  {
    file_save($file, $new);
    verbose 2, "created: $file";
  }
  elsif (contents ($file) eq $new)
  {
    verbose 2, "no changes: $file";
  }
  else
  {
    my $mode = file_mode($file);
    my $mtime = file_mtime($file);
    my $bak = "$file.bak";
    file_rename($file, $bak);
    file_save($file, $new);
    file_chmod($file, $mode);
    file_mtime_set($file, $mtime)
      if $option{mtime};
    file_diff($bak, $file)
      if 2 <= $verbose;
    file_remove($bak)
      if $option{nobak};
  }
}

=item C<file_update_file($file, $src)>

Update the contents of C<$file> to C<$src> if it is different from
the current content.

Same options as above.

=cut

sub file_update_file ($;$%)
{
  my ($file, $src, %option) = @_;

  if ($src eq '')
  {
    $src = "$file.tmp";
  }

  if (! -f $file)
  {
    file_rename($src, $file);
    verbose 2, "created: $file";
  }
  elsif (!file_diff ($file, $src))
  {
    verbose 2, "no changes: $file";
    file_remove($src);
  }
  else
  {
    my $mode = file_mode($file);
    my $mtime = file_mtime($file);
    my $bak = "$file.bak";
    file_rename($file, $bak);
    file_rename($src, $file);
    file_chmod($file, $mode);
    file_mtime_set($file, $mtime)
      if $option{mtime};
    file_remove($bak)
      if $option{nobak};
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

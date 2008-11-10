#! /usr/bin/perl -w
#
# Copyright (C) 2006, 2007, 2008  Benoit Sigoure.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
# USA.

# FIXME: Write some documentation

use strict;
use vars qw($mk_file $rv $n);

# ------------ #
# Input files. #
# ------------ #

my $config_h = 'sdk/config.h';
$mk_file = 'sdk/param.mk';


$n = 0;  # Line number.
$rv = 0; # Return value of this script.

# ensure_file_exists(file-name)
# Makes sure that file-name exists.
# If it doesn't, try to generate it with config.status.
sub ensure_file_exists($)
{
  my $file = shift;
  if (! -r $file)
  {
    if (-x './config.status')
    {
      print `./config.status $file`;
    }
    else
    {
      print STDERR "$0: Error: $file doesn't exist.\n";
      exit 1;
    }
  }
}

ensure_file_exists($config_h);
ensure_file_exists($mk_file);

open CONFIG_H, $config_h or die "Cannot open $config_h: $!";
my %defines;
while(<CONFIG_H>)
{
  next if ($_ !~ /^#[ \t]*define/);
  /^#[ \t]*define[ \t]*(\w+)[^(][ \t]*(.*?)[ \t]*$/;
  $defines{$1} = $2;
}
close CONFIG_H;

open PARAM_MK, $mk_file or die "Cannot open $mk_file: $!";
my @param_mk = <PARAM_MK>;
close PARAM_MK;

# error(error-msg [, line-no])
sub error($;$$)
{
  $rv = 1;
  my ($msg, $line) = @_;
  $line = $n if !defined($line);
  print STDERR "$0:$mk_file:" . ($line + 1)
	       . ': ' . $msg . "\n";
}

open PARAM_MK, ">$mk_file"
  or die "Cannot open $mk_file for writing: $!";
select PARAM_MK or die "internal error: $!";

my $last_if_at_line = -1;
my $nblines = $#param_mk;
for ($n = 0; $n <= $nblines; ++$n)
{
  $_ = $param_mk[$n]; # Set the current line.

  if (/^if[ \t]*/)
  {
    $last_if_at_line = $n;
    (my $condition = $') =~ s/[ \t\r]*$//; # Pacify Emacs'.
    chomp($condition);
    #print STDERR "condition: '$condition'\n";
    if ($condition !~ /^\w+$/)
    {
      error ("Invalid macro name: `$condition'");
      while ($n <= $nblines && $param_mk[$n] !~ /^endif/)
      {
	#print STDERR "skip[error]>$param_mk[$n]";
	++$n;
      }
      next;
    }
    #print STDERR "eat>$param_mk[$n]";
    ++$n;
    if (defined($defines{$condition}) && $defines{$condition} != 0)
    {
      while ($n <= $nblines && $param_mk[$n] !~ /^(endif|else)/)
      {
	#print STDERR "true>$param_mk[$n]";
	print $param_mk[$n];
	++$n;
      }
      if ($n < $nblines && $param_mk[$n] =~ /^else/)
      {
	while ($n <= $nblines && $param_mk[$n] !~ /^endif/)
	{
	  #print STDERR "skip[else]>$param_mk[$n]";
	  ++$n;
	}
      }
    }
    else
    {
      while ($n <= $nblines && $param_mk[$n] !~ /^(endif|else)/)
      {
	#print STDERR "skip[false]>$param_mk[$n]";
	++$n;
      }
      if ($n <= $nblines && $param_mk[$n] =~ /^else/)
      {
	#print STDERR "eat>$param_mk[$n]";
	++$n;
	while ($n <= $nblines && $param_mk[$n] !~ /^endif/)
	{
	  #print STDERR "else>$param_mk[$n]";
	  print $param_mk[$n];
	  ++$n;
	}
      }
    }
    #print STDERR "eat[auto]>$param_mk[$n]";
    if ($param_mk[$n] !~ /^endif/)
    {
      if ($n <= $nblines)
      {
	error ('Missing endif for this if', $last_if_at_line);
      }
      else
      {
	die "internal error at $mk_file:" . ($n + 1);
      }
    }
    $last_if_at_line = -1;
    next;
  }
  #print STDERR "line[$n]: $_";
  print $_;
}

error('Missing endif for this if', $last_if_at_line)
  if ($last_if_at_line != -1);

close PARAM_MK;
exit $rv;

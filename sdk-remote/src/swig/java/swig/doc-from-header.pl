#!/usr/bin/perl

## Copyright (C) 2010-2011, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

# List the headers files you want to parse in argument to this
# script. This script parse C++ header files, retrieve the
# documentation they contains, and generate a SWIG file containing
# this documentation. Then SWIG apply the docummentation to the Java
# code.

my $in = 0;
my $comment = "";
my $code = 1;
my $namespace;
my $print = 0;
my $current_class;
my $private = 0;
my $copydoc = "";
my $cls="";

sub printAndResetCopydoc(){
    # do not print it doesn't work :(
    #print "%typemap(javacode) $current_class \"$copydoc\";\n" if ($copydoc !~ /^$/);
    $copydoc="";
}

sub handleClassComment()
{
    my ($name, $comment) = @_;
    $namespace = "urbi::";
    $namespace .= "impl::" if (/Impl/);
    if ($comment !~ /^$/) {
	print "%typemap(javaimports) ".$namespace.$name."\n\"";
	print $comment."\"\n\n";
    }
    #$copydoc =~ s/::/./g;
    &printAndResetCopydoc();
    $cls = $name;
    $current_class=$namespace.$name

}

sub handleMethodComment()
{
    my ($name, $comment) = @_;
    if ($comment !~ /^$/) {
	$comment =~ s/([^\\])?"/\1\\"/g; # backquote non backquoted quotes
	$comment =~ s/\\([^"])/\\\\\1/g; # double the backslashes
	print "%javamethodmodifiers ${current_class}::$name\"$comment public \";\n";
    }
}

sub handleAttributeComment()
{
    my ($name, $comment) = @_;
    if ($comment !~ /^$/) {
	$comment =~ s/([^\\])?"/\1\\"/g; # backquote non backquoted quotes
	$comment =~ s/\\([^"])/\\\\\1/g; # double the backslashes
	$name =~ s/\n//g;
	$copydoc .= "///! \@copydoc ${current_class}::$name()\n$comment\n"
    }
}


foreach $inputfile (@ARGV) {
    $in = 0;
    $comment = "";
    $code = 1;
    $namespace;
    $print = false;
    $current_class = "urbi";


    open(INPUT,"<$inputfile");
    while(<INPUT>) {

	$code = 1;

	# Test for C-style comments, and save comment if we are in a comment
	# if contains '/*'
	if (/\/\*/) {
	    $code = 0;
	    $in++;
	}
	# if we are in a block /*...*/
	if ($in > 0) {
	    $comment .= $_;
	}
	# if contains '*/'
	if (/\*\//) {
	    $code = 0;
	    $in--;
	}

	# Test for C++ style comment, and save comment if we are in a comment
	# if contains '//' and if line starts by comment
	if ((/^\s*\/\//)) {
	    $code = 0;
	    $comment .= $_;
	}

	$private = 1 if (/private/);
	$private = 0 if ((/public/) || (/protected/));

	# Upon finding a class/method name, print associated comment
	if (($in == 0)) {
	    if (/class URBI_SDK_API/) {
		s/.*class URBI_SDK_API ([^ :]+).*\n/\1/;
		&handleClassComment($_, $comment);
	    }
	    elsif (/^\W+class \w+/) {
		s/.*class (\w+).*\n/\1/;
		&handleClassComment($_, $comment);
	    }
	    elsif ((!/typedef/) && !/using/) {
		if ((/[\w]+\s*\(.*\).*;/)) {
		    s/^[^()]*\s(~*[\w]+)[\b\s]*\(.*\).*/\1/;
		    &handleMethodComment($_, $comment);

		}
		elsif ((/operator\b*[^\b]*\b*\(.*\);/)) {
		    s/.*(operator\b*[^\b]*)\b*\((.*)\);/\1(\2)/;
		    &handleMethodComment($_, $comment);
		}
		elsif ((/\s+[\w:_]+[\w_]+;/) && (!/[()]/) && ($comment !~ /^$/)) {
		    s/\bstatic\b//g;
		    s/\bconst\b//g;
		    s/^\s*\S+\s*([^\s;*,]+).*$/\1/;
		    s/\n//;
		    &handleAttributeComment("set".ucfirst($_)."\n", $comment);
		    &handleAttributeComment("get".ucfirst($_."\n"), $comment);
		}
	    }
	}

	# Reset comment if this line:
	# - is not empty
	# - is not inside a comment block
	# - contains C++ source code
	if (($_ !~ /^$/) && ($in == 0) && ($code == 1)) {
	    $comment = "";
	}
    }
    &printAndResetCopydoc();
    close(INPUT);
}

#! /usr/bin/perl -w 


#
# Template autogenerator.
# This script will generate template code from meta-template code.
# It is designed to avoid having to write multiple versions of a template
# code for various number of arguments.
# Call : template_autogen.pl file
#
# Action: within %%%% n1 n2\n...\n%%%%\n:
#    - expands "%% whatever P%, %%"   to  "whetever P1, whatever P2, ..., whatever PN,"
#    - expands "%%%,% whatever P% %%" to  "whetever P1, whatever P2, ..., whatever PN"
#    - expands "%N%" to number of arguments
#
$file = $ARGV[0];


$DEBUG = 0;
open(DAT, $file);


while(<DAT>) {
    if (/%%%%/) {
	$first = 1;
	$last = 7;
	if (/^%%%%\s*([0-9]+).*?([0-9]+)/) {
	    $first = int($1);
	    $last = int($2);
	}
	
	
	@block=();
	while (<DAT>) {
	    last if (/%%%%/);
	    push (@block , $_);
	    print "PUSHING $_" if ($DEBUG);
	}
	for (my $i=$first+0;$i < $last+1; $i++) {
	    &parseMetaBlock($i, @block);
	}
    }
    else {
	print;
    }
}



sub parseMetaBlock {
    my $num = shift;
    my @block = @_;
    my $in = 0;
    my $line = '';
    my $sep = '';
    my $ln = '';
    
    while($_ = shift @block) {
	s/\n//;
	print "GOT '$_'\n" if ($DEBUG);
	s/\%N%/$num/g;
	$in = 0;
	$sep = '';
	if (/(.*?)%%%([^%]*)%(.*)$/) {
	    print ("XDUMPING '$1'\n") if ($DEBUG);
	    $in = 1;
	    print $1;
	    $_ = $3;
	    $line ='';
	    $sep = $2;
	    
	}
	else {
	    if (/^(.*?)%%(.*)$/) {
		print ("DUMPING '$1'\n") if ($DEBUG);
		$in = 1;
		print $1;
		$_ = $2;
		$line ='';
	    }
	}
	if ($in) {
	    
	    #read til end
	    do  {
		print ("GET '$_'\n") if($DEBUG);
		s/\n//;
		s/\%N%/$num/g;
		$ln = $_;
		if (/^(.*?)%%(.*)$/) {
		    $after=$2;
		    $line = $line.$1;
		    print ("TRANSLATING '$line'\n") if ($DEBUG);
		    #handle
		    for ($i=1;$i<=$num;$i++) {
			$cp = $line;
			$cp =~ s/%/$i/g;
			print $cp;
			print $sep if ($i<$num);
		    }
		    print ("AFTDUMPING '$after'\n") if ($DEBUG);
		    print $after."\n";;
		    $in = 0;
		}
		else {
		    print ("ACCUM '$line'\n") if($DEBUG);
		    $line = $line.$ln."\n";
		}
	    }
	    while($in && ($_ = shift @block));
	}
	else {
	    print "IGNORING '$_'\n" if ($DEBUG);
	print $_."\n";;
	}
	
    }
}

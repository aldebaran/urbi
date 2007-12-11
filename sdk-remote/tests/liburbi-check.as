m4_pattern_allow([^URBI_SERVER$])		         -*- shell-script -*-

AS_INIT()dnl
URBI_PREPARE()

set -e

case $VERBOSE in
  x) set -x;;
esac

# Avoid zombies and preserve debugging information.
cleanup ()
{
  exit_status=$?
  # Signal 0 is confusing, it just means we exited normally.
  test "$1" == 0 ||
    stderr "signal $1"

  # In case we were caught by set -e, kill the children.
  kill_children
  harvest_children

  echo "cleanup finished correctly, exiting $exit_status"
  echo

  rst_subsection "$me: Debug outputs"

  # Results.
  for i in $children
  do
    rst_run_report "$i" "$i"
  done

  exit $exit_status
}
for signal in 0 1 2 13 15; do
  trap "cleanup $signal" $signal
done


# Overriden to include the test name.
stderr ()
{
  echo >&2 "$(basename $0): $me: $@"
  echo >&2
}


# Sleep some time, but taking into account the fact that
# instrumentation slows down a lot.
my_sleep ()
{
  if $INSTRUMENT; then
    sleep $(($1 * 5))
  else
    sleep $1
  fi
}

# find_srcdir WITNESS
# -------------------
find_srcdir ()
{
  # Guess srcdir if not defined.
  if test -z "$srcdir"; then
    # Try to compute it from $0.
    srcdir=$(dirname "$0")
  fi

  # We may change directory.
  srcdir=$(absolute "$srcdir")

  # Check that srcdir is valid.
  test -f "$srcdir/$1" ||
    fatal "cannot find $srcdir/$1: define srcdir"

  echo "$srcdir"
}


## -------------- ##
## Main program.  ##
## -------------- ##

# Make it absolute.
chk=$(absolute "$1")
if test ! -f "$chk.cc"; then
  fatal "no such file: $chk.cc"
fi

period=32
# ./../../../tests/2.x/andexp-pipeexp.chk -> 2.x
medir=$(basename $(dirname "$chk"))
# ./../../../tests/2.x/andexp-pipeexp.chk -> 2.x/andexp-pipeexp
me=$medir/$(basename "$chk" ".cc")
# ./../../../tests/2.x/andexp-pipeexp.chk -> andexp-pipeexp
meraw=$(basename $me)    # MERAW!

# Guess srcdir if not defined.
srcdir=$(find_srcdir "tests.hh")

# $URBI_SERVER
find_urbi_server

# Move to a private dir.
rm -rf $me.dir
mkdir -p $me.dir
cd $me.dir

# Help debugging
set | rst_pre "$me variables"

#compute expected output
sed -n -e 's@//= @@p' $chk.cc >output.exp
touch error.exp
echo 0 >status.exp

#start it
valgrind=$(instrument "server.val")
echo $valgrind $URBI_SERVER -p 0 -w server.port $period >server.cmd
$valgrind $URBI_SERVER -p 0 -w server.port $period >server.out 2>server.err &
register_child server

my_sleep 2


#start the test
valgrind=$(instrument "remote.val")
echo $valgrind ../../tests localhost $(cat server.port) $meraw >remote.cmd
$valgrind ../../tests localhost $(cat server.port) $meraw>remote.out 2>remote.err &
register_child remote
my_sleep 1
#wait for...
harvest_children
kill_children
estatus=$?
sleep 1
#generate traces

# If the return value >126, something really wrong is going on.  127 is
# command not found (should not happen here) and >127 means that the
# program was killed by a signal (which often indicates that something
# really wrong is going on).
if test $estatus -gt 126; then
  stderr "Fatal: return value $estatus > 126"
  exit='exit 177' # 177 = hard error
fi

# Debugging data often explains failures, so it should be first.
#if test -s debug.rst; then
#  cat debug.rst
#fi

 # Compare expected output with actual output.
cp remote.out remote.out.eff
rst_expect output remote.out
rst_pre "Error output" remote.err
#cp remote.sta remote.sta.eff
#echo 0 >status.exp
#rst_expect status remote.sta

# Display Valgrind report.
rst_pre "Valgrind" remote.val

echo "$exit" >exit
$exit

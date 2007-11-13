m4_pattern_allow([^URBI_SERVER$])		         -*- shell-script -*-

AS_INIT()dnl
URBI_PREPARE()
URBI_RST_PREPARE()
URBI_INSTRUMENT_PREPARE()

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

  echo "cleanup finished correctly, exiting $exit_status" >>debug
  rst_subsection "$me: Debug outputs" >&3
  rst_pre "debug" debug >&3

  # Results.
  for i in $children
  do
    rst_pre "$i command"  "$i.cmd"
    rst_pre "$i status"   "$i.sta"
    rst_pre "$i output"   "$i.out"
    rst_pre "$i error"    "$i.err"
    rst_pre "$i valgrind" "$i.val"
  done >&3

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


# run TITLE COMMAND...
# --------------------
# Run the COMMAND... (which may use its stdin) and output detailed
# logs about the execution.
#
# Leave un $run_prefix.{cmd, out, sta, err, val} the command, standard
# output, exit status, standard error, and instrumentation output.
run_counter=0
run_prefix=
run ()
{
  local title=$1
  run_prefix=$run_counter-$(echo $1 |
			     sed -e 's/[[^a-zA-Z0-9][^a-zA-Z0-9]]*/-/g;s/-$//')
  run_counter=$((run_counter + 1))
  shift

  {
    ("$@") >$run_prefix.out 2>$run_rst_prefix.err
    local sta=$?
    echo $sta >$run_prefix.sta

    case $sta in
	0) ;;
	*) title="$title FAIL ($sta)";;
    esac
    rst_subsection "$me: $title"

    echo
    echo "$@"> $run_prefix.cmd
    rst_pre "Command"   $run_prefix.cmd
    rst_pre "Output"    $run_prefix.out
    rst_pre "Status"    $run_prefix.sta
    rst_pre "Error"     $run_prefix.err
    rst_pre "Valgrind"  $run_prefix.val

    return $(cat $run_prefix.sta)
  } >&3
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

# find_urbi_server
# ----------------
# Set URBI_SERVER to the location of urbi-server.
find_urbi_server ()
{
  case $URBI_SERVER in
    '')
       # If URBI_SERVER is not defined, try to find one.  If we are in
       # $top_builddir/tests/TEST.dir, then look in $top_builddir/src.
       URBI_SERVER=$(find_prog "urbi-server" "$top_builddir/src${path_sep}.")
       ;;

    *[[\\/]]* ) # A path, relative or absolute.  Make it absolute.
       URBI_SERVER=$(absolute "$URBI_SERVER")
       ;;

    *) # A simple name, most certainly urbi-server.
       # Find it in the PATH.
       URBI_SERVER=$(find_prog "urbi-server" "$PATH")
       # If we can't find it, then ucore-pc was probably not installed.
       # Skip.
       test x"$URBI_SERVER" != x || exit 77
       ;;
  esac

  if test -z "$URBI_SERVER"; then
    fatal "cannot find urbi-server, please define URBI_SERVER"
  fi

  if test ! -f "$URBI_SERVER"; then
    fatal "cannot find urbi-server, please check \$URBI_SERVER: $URBI_SERVER"
  fi

  # Check its version.
  if ! run "Server version" "$URBI_SERVER" --version; then
    fatal "cannot run $URBI_SERVER --version"
  fi
}



## ---------- ##
## Children.  ##
## ---------- ##

# register_child NAME
# -------------------
register_child ()
{
  children="$children $1"
  echo $! >$1.pid
}


# kill_children
# -------------
# Kill all the children.  This function can be called twice: once
# before cleaning the components, and once when exiting.
kill_children ()
{
  for i in $children
  do
    pid=$(cat $i.pid)
    if ps $pid 2>&1; then
      echo "Killing $i ($pid)"
      kill $pid 2>&1 || true
    fi
  done >>debug
}


# harvest_children
# ----------------
# Report the exit status of the children.
harvest_children ()
{
  # Harvest exit status.
  for i in $children
  do
    pid=$(cat $i.pid)
    # Beware of set -e.
    if wait $pid 2>&1; then
      sta=$?
    else
      sta=$?
    fi
    echo "$sta$(ex_to_string $sta)" >$i.sta
  done
}

# find_srcdir
# -----------
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
  test -f "$srcdir/tests.hh" ||
    fatal "cannot find $srcdir/tests.hh: define srcdir"

  echo "$srcdir"
}

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
srcdir=$(find_srcdir)
# $URBI_SERVER
find_urbi_server

# Help debugging
set | rst_pre "$me variables" >&3

# Move to a private test directory.
rm -rf $me.dir
mkdir -p $me.dir
cd $me.dir

exec 3>debug.rst

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
  if test -s debug.rst; then
    cat debug.rst
  fi

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

                                                     -*- shell-script -*-
URBI_INIT

# The full path to the *.uob dir:
# ../../../../../sdk-remote/src/tests/uobjects/access-and-change/uaccess.uob
uob=$(absolute $1)
require_file "$uob"

# For error messages.
me=$(basename "$uob" .uob)

# The directory we work in.
# The ending part, for our builddir: access-and-change/uaccess.dir.
builddir=$(echo "$uob" |
           sed -e 's,.*/src/uobjects/,uobjects/,').dir
mkcd $builddir

# Find urbi-launch.
urbi_launch=$(xfind_prog urbi-launch$EXEEXT)

# If urbi-launch cannot work because there is no kernel libuobject,
# skip the test.  Do pass something that will fail to be loaded,
# otherwise if it works, a server is launched, and will run endlessly.
run "urbi-launch --start" $urbi_launch --start /dev/null ||
  case $? in
    (72) error SKIP "urbi-launch cannot find libuobject";;
  esac

for ext in '' .la $SHLIBEXT
do
  xrun "urbi-launch $me$ext --version" "$urbi_launch" --start $uob$ext -- --version
done

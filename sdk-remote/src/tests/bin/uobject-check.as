                                                     -*- shell-script -*-
URBI_INIT

# The full path to the *.uob dir:
# ../../../../../sdk-remote/src/tests/uobjects/access-and-change/uaccess.uob
uob=$(absolute $1.uob)
require_dir "$uob"

# The ending part, for our builddir: access-and-change/uaccess.dir.
builddir=$(echo "$uob" |
           sed -e 's,.*/src/tests/uobjects/,uobjects/,;s/\.uob$//').dir

# For error messages.
me=$(basename "$uob" .uob)

# The directory we work in.
rm -rf $builddir
mkdir -p $builddir
cd $builddir

# The remote component: an executable.
umake_remote=$(xfind_prog "umake-remote")
xrun "umake-remote" $umake_remote --output=$me$EXEEXT $uob
test -x "$me$EXEEXT" ||
  fatal "$me is not executable"
xrun "$me --version" "./$me$EXEEXT" --version

# The shared component: a dlopen module.
umake_shared=$(xfind_prog "umake-shared")
xrun "umake-shared" $umake_shared --output=$me $uob
test -f "$me.la" ||
  fatal "$me.la does not exist"

# Find urbi-launch.
urbi_launch=$(xfind_prog urbi-launch$EXEEXT)

# If urbi-launch cannot work because there is no kernel libuobject,
# skip the test.  Do pass something that will fail to be loaded,
# otherwise if it works, a server is launched, and will run endlessly.
run "urbi-launch --start" $urbi_launch --start /dev/null ||
  case $? in
    (72) error 77 "urbi-launch cannot find libuobject";;
  esac
xrun "urbi-launch $me --version" "$urbi_launch" --start $me.la -- --version

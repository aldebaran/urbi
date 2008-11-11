m4_pattern_allow([^URBI_(PATH|SERVER)$])         -*- shell-script -*-
URBI_INIT

# The full path to the *.uob dir:
# ../../../../../sdk-remote/src/tests/uobjects/access-and-change/uaccess.uob
uob=$(absolute $1.uob)
test -d "$uob" ||
  error OSFILE "argument is not a directory: $uob"

# The ending part, for our builddir: access-and-change/uaccess.dir.
builddir=$(echo "$uob" |
           sed -e 's,.*/src/tests/uobjects/,uobjects/,;s/\.uob$//').dir

# For error messages.
me=$(basename "$uob" .uob)

# The directory we work in.
rm -rf $builddir
mkdir -p $builddir
cd $builddir

# The remote component name.
remote=urbi-$me

umake_remote=$(find_prog "umake-remote" "$PATH")
test -n "$umake_remote" ||
  error OSFILE "cannot find umake-remote in $PATH"

run "compile \"$remote\"" $umake_remote --output=$remote $uob ||
  fatal "$umake_remote failed"

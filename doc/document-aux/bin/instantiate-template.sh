#! /bin/sh

me=$(basename $0)

stderr ()
{
  for i
  do
    echo >&2 "$me: $i"
  done
}

fatal ()
{
  stderr "$@"
  exit 1
}

usage ()
{
  cat <<EOF
$me: BIBTEX-KEY SUBJECT EMAILS [TEMPLATEDIR] 

with
  KEY          directory name following our conventions FIRST-AUTHOR.YY.CONF
               e.g., "bardeche.07.oopsla"
  SUBJECT      short conf name to use in the messages
               e.g. "OOPSLA'07"
  EMAILS       space separated list of notified authors
               e.g., "bardeche@epita.fr courtois@epita.fr"
  TEMPLATEDIR  template directory [default: template]
EOF
}

if test $# -ne 3 && test $# -ne 4; then
  usage
  exit 1
fi

key=$1
subject=$2
emails=$3

if test $# -eq 4; then
  templatedir=$4
else
  templatedir=template
fi

# Try to guess which Source Control Management (SCM) system is in use here.
if svn info >/dev/null 2>/dev/null; then
  scm=svn
elif git ls-files >/dev/null 2>/dev/null; then
  scm=git
else
  fatal "not a Subversion nor a Git repository."
fi

test ! -d "$key" ||
  fatal "$key exists already"

case scm in
  svn) make -C "$templatedir" share-up ||
         fatal "cannot update share in $templatedir/" ;;
  git) make -C "$templatedir" git-share-up ||
         fatal "cannot update share in $templatedir/" ;;
esac

make -C "$templatedir" ||
  fatal "cannot in $templatedir/"

make -C "$templatedir" clean ||
  fatal "cannot clean $templatedir/"

case $scm in
  svn) svn cp "$templatedir" $key ||
         fatal "cannot cp $templatedir/"
  ;;
  git) (cd "$templatedir" && tar cf - $(git ls-files)) \
         | (mkdir $key && cd $key && tar xf -) \
       || fatal "cannot cp $templatedir/"
       # Subscribe the new directory to share/.
       # FIXME: Instead of using a harcoded path to this (single)
       # `share' submodule, we should query the template about its
       # submodules, and call git-submodules for each of them.
       rm -rf "$key/share"
       prefix=$(git rev-parse --show-prefix)
       abs_share_dir=$(cd "$templatedir/share" && pwd)
       (cd $(git rev-parse --show-cdup) \
          && $abs_share_dir/bin/git-submodules -s share "$prefix/$key")
       git add $key
  ;;
esac

# Remove temporary files.
rm -f $key/*~ $key/,* ||
  fatal "cannot remove temporary files from $key/"

# Don't use the template's ChangeLog.
case $scm in
  svn)
    # We still keep the ChangeLog's history because of svn cp.
    # I don't know how to detach it for real.
    svn rm --force $key/ChangeLog
  ;;
  git) git rm --force $key/ChangeLog ;;
esac
cat >$key/ChangeLog <<EOF
$(date +%F)  $FULLNAME  <$EMAIL>

	Initial creation.

EOF
case $scm in
  svn) svn add $key/ChangeLog ;;
  git) git add $key/ChangeLog ;;
esac

key=$key emails=$emails subject=$subject \
  perl -pi -e 's/\@KEY\@/$ENV{key}/g;'         \
           -e 's/\@EMAILS\@/$ENV{emails}/g;'   \
           -e 's/\@SUBJECT\@/$ENV{subject}/g;' \
    $key/article.tex $key/Makefile $key/vcs/local.rb

case $scm in
  svn)
    # In the template and svn:ignore, the email address, topic, etc. are
    # already filled in for the template itself.
    svn propget svn:ignore $key >$key/ignore.svn
    perl -pi -e "s'lrde@lrde.epita.fr'$emails'g;"   \
             -e "s<\"template <><\"$subject >g;" 	 \
        $key/vcs/local.rb $key/ignore.svn
    svn propset svn:ignore $key -F $key/ignore.svn
    rm $key/ignore.svn
  ;;
  git)
    perl -pi -e "s'lrde@lrde.epita.fr'$emails'g;"   \
             -e "s<\"template <><\"$subject >g;" 	 \
        $key/vcs/local.rb
  ;;
esac

make -C $key ||
  fatal "cannot compile in $key"

stderr "$key seems ready to work with, please check it in"
exit 0

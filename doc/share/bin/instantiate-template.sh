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
$me: BIBTEX-KEY SUBJECT EMAILS

with
  KEY      directory name following our conventions FIRST-AUTHOR.YY.CONF
	   e.g., "bardeche.07.oopsla"
  SUBJECT  short conf name to use in the messages
	   e.g. "OOPSLA'07"
  EMAILS   space separated list of notified authors
	   e.g., "bardeche@epita.fr courtois@epita.fr"
EOF
}

if test $# != 3; then
  usage
  exit 1
fi

key=$1
subject=$2
emails=$3

test ! -d "$key" ||
  fatal "$key exists already"

make -C template share-up ||
  fatal "cannot update share in template/"

make -C template ||
  fatal "cannot in template/"

make -C template clean ||
  fatal "cannot clean template/"

svn cp template $key ||
  fatal "cannot cp template/"

# Remove temporary files.
rm -f $key/*~ $key/,* ||
  fatal "cannot remove temporary files from $key/"

# Don't use the template's ChangeLog.  We still keep its history
# because of svn cp.  I don't know how to detach it for real.
svn rm --force $key/ChangeLog
cat >$key/ChangeLog <<EOF
$(date +%F)  $FULLNAME  <$EMAIL>

	Initial creation.

EOF
svn add $key/ChangeLog

perl -pi -e "s<\@KEY\@><$key>g;"         \
         -e "s<\@EMAILS\@><$emails>g;"   \
         -e "s<\@SUBJECT\@><$subject>g;" \
    $key/article.tex $key/Makefile $key/vcs/local.rb

# In the template and svn:ignore, the email address, topic, etc. are
# already filled in for the template itself.
svn propget svn:ignore $key >$key/ignore.svn
perl -pi -e "s<lrde\@lrde.epita.fr><$emails>g;"   \
         -e "s<\"template <><\"$subject >g;" 	 \
    $key/vcs/local.rb $key/ignore.svn
svn propset svn:ignore $key -F $key/ignore.svn
rm $key/ignore.svn

make -C $key ||
  fatal "cannot compile in $key"

stderr "$key seems ready to work with, please check it in"
exit 0

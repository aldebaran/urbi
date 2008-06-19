#!/bin/sh

if test $# -lt 2; then
    echo "
Usage: ./gen_biblio.sh key-pattern bibfile1 bibfile2 ... > biblio.bib

  This program is used to generate a small bib file extracted from
  a list of bib files.

examples:
  \$ $0 'sigoure\..*\.seminar' share/bib/*.bib > sigoure-seminar.bib
  \$ $0 'pierron\|moulard' share/bib/*.bib
"
    exit 0;
fi

pattern=$1
shift

if which bibtool > /dev/null; then
    # TODO: remove the @PREAMBLE commands.
    bibtool -X ${pattern} $@
else # emulate it
    cat $@ | sed -n "
/^@.*{[         ]*.*\(${pattern}\).*,/ {
  p;
  :begin;
  n;
  p;
  s/^}/}/;
  T begin;
}
"
fi

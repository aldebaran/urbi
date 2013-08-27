#! /bin/bash

set -e

mkdir -p build-doc
cd build-doc
rm -rf urbi-sdk.htmldir.tmp
mkdir -p urbi-sdk.htmldir.tmp


MAKEINDEX="makeindex -s headings.ist" TEXINDY="texindy --module makeindex" TEX4HT="tex4ht -cunihtf -utf8" ../doc/document-aux/bin/texi2dvi --html --tidy --build-dir=tmp.t2d --batch -I ../doc/document-aux/styles -I ../doc/document-aux/bib -I ../doc/document-aux/ -I ../doc --tex4ht --tex4ht-options=urbi-style,2 --verbose --debug -o urbi-sdk.htmldir.tmp/urbi-sdk.html ../doc/urbi-sdk.tex

cd urbi-sdk.htmldir.tmp                                 \

../../doc/document-aux/bin/tex4ht-post --index urbi-sdk.html urbi-sdk*.html


cp ../../doc/document-aux/figs/doc-* .
cp ../../doc/document-aux/figs/urbi-sdk/2.7/cube-and-disc.png urbi-sdk0x.png
cp ../../doc/document-aux/figs/gostai.png urbi-sdk1x.png
cp ../../doc/img/urbi-architecture.png urbi-sdk2x.png
cd ..
rm -rf urbi-sdk.htmldir
mv urbi-sdk.htmldir.tmp urbi-sdk.htmldir

MAKEINDEX="makeindex -s headings.ist" TEXINDY="texindy --module makeindex" TEX4HT="tex4ht -cunihtf -utf8" ../doc/document-aux/bin/texi2dvi --pdf --tidy --build-dir=tmp.t2d --batch  -I ../doc/document-aux/styles -I ../doc/document-aux/bib -I ../doc/document-aux/ -I ../doc -o urbi-naming.pdf ../doc/urbi-naming.tex

MAKEINDEX="makeindex -s headings.ist" TEXINDY="texindy --module makeindex" TEX4HT="tex4ht -cunihtf -utf8" ../doc/document-aux
/bin/texi2dvi --pdf --tidy --build-dir=tmp.t2d --batch  -I ../doc/document-aux/styles -I ../doc/document-aux/bib -I ../doc/document-aux/ -I ../doc -o urbi-sdk.pdf ../doc/urbi-sdk.tex

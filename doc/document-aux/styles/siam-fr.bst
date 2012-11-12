%%
%% Bib. style "siam-fr" (version francisee de siam.bst)
%%
%% NM, 2004/03/05
%% markey@lsv.ens-cachan.fr 
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Original Copyright (Oren Patashnik & Howard Trickey):
%%
 % SIAM bibliography style (29-Jan-88 version)
 %    numeric labels, alphabetic order, Mathematical Reviews abbreviations,
 %    names in \sc, titles in italics, book titles mixed upper-lower and article
 %    titles lowercase, commas separate all fields except before "notes".
 %
 %   History
 %    1/30/86	(HWT)	Original version, by Howard Trickey.
 %    6/15/87	(HWT)	Fix format.editors---Martin Costabel.
 %    1/29/88	(OP&HWT) Updated for BibTeX version 0.99a, Oren Patashnik;
 %			THIS `siam' VERSION DOES NOT WORK WITH BIBTEX 0.98i.
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

ENTRY
  { address
    author
    booktitle
    chapter
    edition
    editor
    howpublished
    institution
    journal
    key
    month
    note
    number
    organization
    pages
    publisher
    school
    series
    title
    type
    volume
    year
  }
  {}
  { label }

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% C'est ici que je definis les "traductions". Normalement, y a 
%% que ca a changer pour franciser le style...
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


FUNCTION{fr.and}{		%% le "et" entre les deux derniers auteurs
" et "
}

FUNCTION{fr.editeur}{		%% ", \'editeur" au singulier, ou " (\'editeur)"...
", \'ed."
}

FUNCTION{fr.editeurs}{		%% ", \'editeurs" au pluriel, ou " (\'editeurs)"...
", \'eds."
}

FUNCTION{fr.et.al}{		%% " et~al."
" \emph{et~al.}"
}

FUNCTION{fr.in}{		%% "Dans "
"\emph{In} "
}

FUNCTION{fr.in.min}{		%% "Dans "
"\emph{in} "
}

FUNCTION{fr.in.spc}{		%% " de " (ou " de la s\'erie ")
" de "
}

FUNCTION{fr.of}{		%% " de "
" de "
}

FUNCTION{fr.number}{		%% "num{\'e}ro "
"no~"
}

FUNCTION{fr.number.maj}{	%% "Num\'ero "
"No~"
}

FUNCTION{fr.volume}{		%% "volume"
"vol.~"
}

FUNCTION{fr.volume.maj}{	%% "Volume"
"Vol.~"
}

FUNCTION{fr.edition}{		%% " \'edition"
"~\'ed."
}

FUNCTION{fr.pages}{		%% "pages"
"p.~"
}

FUNCTION{fr.page}{		%% "page"
"p~"
}

FUNCTION{fr.chapter}{		%% "chapitre"
"chap.~"
}

FUNCTION{fr.tech.rep}{		%% "Rapport Technique"
"Rap. tech."
}

FUNCTION{fr.master}{		%% "M\'emoire de D.E.A."
"M\'em. D.E.A."
}

FUNCTION{fr.phd}{		%% "Th\`ese de doctorat"
"Th\`ese doctorat"
}

FUNCTION{fr.auteurs.style}{	
	%% si on veut des small caps sur le LASTNAME de l'auteur
	%% Cette fonction est utilisee dans la definition d'une
	%% fonction LaTeX.
"\scshape"
}

FUNCTION{fr.ponctuation.apres.auteurs}{ %% Comme son nom l'indique...
					%% Laisser vide pour un "."
" : "
}

MACRO {jan} {"jan."}
MACRO {feb} {"f\'ev."}
MACRO {mar} {"mars"}
MACRO {apr} {"avril"}
MACRO {may} {"mai"}
MACRO {jun} {"juin"}
MACRO {jul} {"juil."}
MACRO {aug} {"ao\^ut"}
MACRO {sep} {"sept."}
MACRO {oct} {"oct."}
MACRO {nov} {"nov."}
MACRO {dec} {"d\'ec."}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% La suite, normalement, y a rien a changer...
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


INTEGERS { output.state before.all mid.sentence after.block after.authors }

FUNCTION {init.state.consts}
{ #0 'before.all :=
  #1 'mid.sentence :=
  #2 'after.block :=
  #0 'after.authors :=
}

STRINGS { s t }

FUNCTION{ fr.add.period }{
after.authors 
  { fr.ponctuation.apres.auteurs empty$
      { add.period$ }
      { fr.ponctuation.apres.auteurs * #0 'after.authors := }
    if$}
  { add.period$ }
if$
}

FUNCTION {output.nonnull}
{ 's :=
  output.state mid.sentence =
    { ", " * write$ }
    { output.state after.block =
	{ fr.add.period write$
	  newline$
	  "\newblock " write$
	}
	'write$
      if$
      mid.sentence 'output.state :=
    }
  if$
  s
}

FUNCTION {output}
{ duplicate$ empty$
    'pop$
    'output.nonnull
  if$
}

FUNCTION {output.check}
{ 't :=
  duplicate$ empty$
    { pop$ "empty " t * " in " * cite$ * warning$ }
    'output.nonnull
  if$
}

FUNCTION {output.bibitem}
{ newline$
  "\bibitem{" write$
  cite$ write$
  "}" write$
  newline$
  ""
  before.all 'output.state :=
}

FUNCTION {fin.entry}
{ add.period$
  write$
  newline$
}

FUNCTION {new.block}
{ output.state before.all =
    'skip$
    { after.block 'output.state := }
  if$
}

FUNCTION {not}
{   { #0 }
    { #1 }
  if$
}

FUNCTION {and}
{   'skip$
    { pop$ #0 }
  if$
}

FUNCTION {or}
{   { pop$ #1 }
    'skip$
  if$
}

FUNCTION {new.block.checka}
{ empty$
    'skip$
    'new.block
  if$
}

FUNCTION {field.or.null}
{ duplicate$ empty$
    { pop$ "" }
    'skip$
  if$
}

FUNCTION {emphasize}
{ duplicate$ empty$
    { pop$ "" }
    { "{\em " swap$ * "}" * }
  if$
}

FUNCTION {scapify}
{ duplicate$ empty$
    { pop$ "" }
    { "{\fonteauteurs{" swap$ * "}}" * }
  if$
}

INTEGERS { nameptr namesleft numnames }

FUNCTION {format.names}
{ 's :=
  #1 'nameptr :=
  s num.names$ 'numnames :=
  numnames 'namesleft :=
    { namesleft #0 > }
    { s nameptr "{f.~}{vv~}{ll}{, jj}" format.name$ 't :=
	%% Le passage en small-caps est fait plus loin, sur tout le nom.
	%% (c'est le comportement du siam.bst original)
      nameptr #1 >
	{ namesleft #1 >
	    { ", " * t * }
	    { %numnames #2 >
		%{ "," * }
		%'skip$
	      %if$
	      t "others" =
		{ fr.et.al * }
		{ fr.and * t * }
	      if$
	    }
	  if$
	}
	't
      if$
      nameptr #1 + 'nameptr :=
      namesleft #1 - 'namesleft :=
    }
  while$
}

STRINGS { last.authors }

FUNCTION {init.last.authors}
{ "" 'last.authors :=
}

FUNCTION {format.authors}
{ author empty$
    { "" 'last.authors :=
      ""
    }
    { author last.authors =
        { "\leavevmode\vrule height 2pt depth -1.6pt width 23pt" }
        { author format.names scapify }
      if$
      author 'last.authors :=
    }
  if$
}

FUNCTION {format.organization}
{ organization empty$
    { "" 'last.authors :=
      ""
    }
    { organization last.authors =
        { "\leavevmode\vrule height 2pt depth -1.6pt width 23pt" }
        { organization scapify }
      if$
      organization 'last.authors :=
    }
  if$
}

FUNCTION {format.editors}
{ editor empty$
    { "" 'last.authors :=
      ""
    }
    { editor last.authors =
        { "\leavevmode\vrule height 2pt depth -1.6pt width 23pt" }
	{ editor format.names scapify }
      if$
      editor num.names$ #1 >
        { fr.editeurs * }
	{ fr.editeur * }
      if$
      editor 'last.authors :=
    }
  if$
}

FUNCTION {format.ineditors}
{ editor empty$
    { "" }
    { editor format.names
      editor num.names$ #1 >
	{ fr.editeurs * }
	{ fr.editeur * }
      if$
    }
  if$
}

FUNCTION {format.title}
{ title empty$
    { "" }
    { title "t" change.case$ emphasize }
  if$
}

FUNCTION {n.dashify}
{ 't :=
  ""
    { t empty$ not }
    { t #1 #1 substring$ "-" =
	{ t #1 #2 substring$ "--" = not
	    { "--" *
	      t #2 global.max$ substring$ 't :=
	    }
	    {   { t #1 #1 substring$ "-" = }
		{ "-" *
		  t #2 global.max$ substring$ 't :=
		}
	      while$
	    }
	  if$
	}
	{ t #1 #1 substring$ *
	  t #2 global.max$ substring$ 't :=
	}
      if$
    }
  while$
}

FUNCTION {format.date}
{ year empty$
    { month empty$
	{ "" }
	{ "there's a month but no year in " cite$ * warning$
	  month
	}
      if$
    }
    { month empty$
	'year
	{ month " " * year * }
      if$
    }
  if$
}

FUNCTION {format.btitle}
{ title emphasize
}

FUNCTION {tie.or.space.connect}
{ duplicate$ text.length$ #3 <
    { "~" }
    { " " }
  if$
  swap$ * *
}

FUNCTION {either.or.check}
{ empty$
    'pop$
    { "can't use both " swap$ * " fields in " * cite$ * warning$ }
  if$
}

FUNCTION {format.bvolume}
{ volume empty$
    { "" }
    { fr.volume volume *
      series empty$
	'skip$
	{ fr.of * series * }
      if$
      "volume and number" number either.or.check
    }
  if$
}

FUNCTION {format.number.series}
{ volume empty$
    { number empty$
	{ series field.or.null }
	{ fr.number number *
	  series empty$
	    { "there's a number but no series in " cite$ * warning$ }
	    { fr.in.min * series * }
	  if$
	}
      if$
    }
    { "" }
  if$
}

FUNCTION {format.edition}
{ edition empty$
    { "" }
    { edition "l" change.case$ fr.edition * }
  if$
}

INTEGERS { multiresult }

FUNCTION {multi.page.check}
{ 't :=
  #0 'multiresult :=
    { multiresult not
      t empty$ not
      and
    }
    { t #1 #1 substring$
      duplicate$ "-" =
      swap$ duplicate$ "," =
      swap$ "+" =
      or or
	{ #1 'multiresult := }
	{ t #2 global.max$ substring$ 't := }
      if$
    }
  while$
  multiresult
}

FUNCTION {format.pages}
{ pages empty$
    { "" }
    { pages multi.page.check
	{ fr.pages pages n.dashify * }
	{ fr.page pages * }
      if$
    }
  if$
}

FUNCTION {format.vol.year}
{ volume field.or.null
  year empty$
    { "empty year in " cite$ * warning$ }
    { " (" year * ")" * * }
  if$
}

FUNCTION {format.chapter.pages}
{ chapter empty$
    'format.pages
    { type empty$
	{ fr.chapter chapter * }
	{ type "l" change.case$ chapter tie.or.space.connect }
      if$
      pages empty$
	'skip$
	{ ", " * format.pages * }
      if$
    }
  if$
}

FUNCTION {format.in.ed.booktitle}
{ booktitle empty$
    { "" }
    { editor empty$
	{ fr.in.min booktitle * }
	{ fr.in.min booktitle * ", " * format.ineditors * }
      if$
    }
  if$
}

FUNCTION {empty.misc.check}
{ author empty$ title empty$ howpublished empty$
  month empty$ year empty$ note empty$
  and and and and and
  key empty$ not and
    { "all relevant fields are empty in " cite$ * warning$ }
    'skip$
  if$
}

FUNCTION {format.thesis.type}
{ type empty$
    'skip$
    { pop$
      type "l" change.case$
    }
  if$
}

FUNCTION {format.tr.number}
{ type empty$
    { fr.tech.rep }
    'type
  if$
  number empty$
    { "l" change.case$ }
    { number tie.or.space.connect }
  if$
}

FUNCTION {format.article.crossref}
{ key empty$
    { journal empty$
	{ "need key or journal for " cite$ * " to crossref " * crossref *
	  warning$
	  ""
	}
	{ fr.in.min journal * }
      if$
    }
    { fr.in.min key * }
  if$
  " \cite{" * crossref * "}" *
}

FUNCTION {format.crossref.editor}
{ editor #1 "{vv~}\bgroup\fonteauteurs\bgroup{ll}\egroup\egroup{{}}" format.name$
  editor num.names$ duplicate$
  #2 >
    { pop$ fr.et.al * }
    { #2 <
	'skip$
	{ editor #2 "{ff }{vv }{ll}{ jj}" format.name$ "others" =
	    { fr.et.al * }
	    { fr.and * editor #2 "{vv~}\bgroup\fonteauteurs\bgroup{ll}\egroup\egroup{{}}" format.name$ * }
	  if$
	}
      if$
    }
  if$
}

FUNCTION {format.book.crossref}
{ volume empty$
    { "empty volume in " cite$ * "'s crossref of " * crossref * warning$
      fr.in.min
    }
    { fr.volume volume *
      fr.of *
    }
  if$
  editor empty$
  editor field.or.null author field.or.null =
  or
    { key empty$
	{ series empty$
	    { "need editor, key, or series for " cite$ * " to crossref " *
	      crossref * warning$
	      "" *
	    }
	    { series * }
	  if$
	}
	{ key * }
      if$
    }
    { format.crossref.editor * }
  if$
  " \cite{" * crossref * "}" *
}

FUNCTION {format.incoll.inproc.crossref}
{ editor empty$
  editor field.or.null author field.or.null =
  or
    { key empty$
	{ booktitle empty$
	    { "need editor, key, or booktitle for " cite$ * " to crossref " *
	      crossref * warning$
	      ""
	    }
	    { fr.in.min booktitle * }
	  if$
	}
	{ fr.in.min key * }
      if$
    }
    { fr.in.min format.crossref.editor * }
  if$
  " \cite{" * crossref * "}" *
}

FUNCTION {article}
{ output.bibitem
  format.authors "author" output.check
  format.title "title" output.check
  crossref missing$
    { journal "journal" output.check
      format.vol.year output
    }
    { format.article.crossref output.nonnull }
  if$
  format.pages output
  new.block
  note output
  fin.entry
}

FUNCTION {book}
{ output.bibitem
  author empty$
    { format.editors "author and editor" output.check }
    { format.authors output.nonnull
      crossref missing$
	{ "author and editor" editor either.or.check }
	'skip$
      if$
    }
  if$
  format.btitle "title" output.check
  crossref missing$
    { format.bvolume output
      format.number.series output
      publisher "publisher" output.check
      address output
    }
    { format.book.crossref output.nonnull }
  if$
  format.edition output
  format.date "year" output.check
  new.block
  note output
  fin.entry
}

FUNCTION {booklet}
{ output.bibitem
  format.authors output
  format.title "title" output.check
  howpublished new.block.checka
  howpublished output
  address output
  format.date output
  new.block
  note output
  fin.entry
}

FUNCTION {inbook}
{ output.bibitem
  author empty$
    { format.editors "author and editor" output.check }
    { format.authors output.nonnull
      crossref missing$
	{ "author and editor" editor either.or.check }
	'skip$
      if$
    }
  if$
  format.btitle "title" output.check
  crossref missing$
    { format.bvolume output
      format.number.series output
      publisher "publisher" output.check
      address output
    }
    { format.book.crossref output.nonnull }
  if$
  format.edition output
  format.date "year" output.check
  format.chapter.pages "chapter and pages" output.check
  new.block
  note output
  fin.entry
}

FUNCTION {incollection}
{ output.bibitem
  format.authors "author" output.check
  format.title "title" output.check
  crossref missing$
    { format.in.ed.booktitle "booktitle" output.check
      format.bvolume output
      format.number.series output
      publisher "publisher" output.check
      address output
      format.edition output
      format.date "year" output.check
    }
    { format.incoll.inproc.crossref output.nonnull }
  if$
  format.chapter.pages output
  new.block
  note output
  fin.entry
}

FUNCTION {inproceedings}
{ output.bibitem
  format.authors "author" output.check
  format.title "title" output.check
  crossref missing$
    { format.in.ed.booktitle "booktitle" output.check
      format.bvolume output
      format.number.series output
      address empty$
	{ organization output
	  publisher output
	  format.date "year" output.check
	}
	{ address output.nonnull
	  format.date "year" output.check
	  organization output
	  publisher output
	}
      if$
    }
    { format.incoll.inproc.crossref output.nonnull }
  if$
  format.pages output
  new.block
  note output
  fin.entry
}

FUNCTION {conference} { inproceedings }

FUNCTION {manual}
{ output.bibitem
  author empty$
    { format.organization output }
    { format.authors output.nonnull }
  if$
  format.btitle "title" output.check
  author empty$
    'skip$
    { organization output }
  if$
  address output
  format.edition output
  format.date output
  new.block
  note output
  fin.entry
}

FUNCTION {mastersthesis}
{ output.bibitem
  format.authors "author" output.check
  format.title "title" output.check
  fr.master format.thesis.type output.nonnull
  school "school" output.check
  address output
  format.date "year" output.check
  new.block
  note output
  fin.entry
}

FUNCTION {misc}
{ output.bibitem
  format.authors output
  format.title output
  howpublished new.block.checka
  howpublished output
  format.date output
  new.block
  note output
  fin.entry
  empty.misc.check
}

FUNCTION {phdthesis}
{ output.bibitem
  format.authors "author" output.check
  format.btitle "title" output.check
  fr.phd format.thesis.type output.nonnull
  school "school" output.check
  address output
  format.date "year" output.check
  new.block
  note output
  fin.entry
}

FUNCTION {proceedings}
{ output.bibitem
  editor empty$
    { format.organization output }
    { format.editors output.nonnull }
  if$
  format.btitle "title" output.check
  format.bvolume output
  format.number.series output
  address empty$
    { editor empty$
	'skip$
	{ organization output }
      if$
      publisher output
      format.date "year" output.check
    }
    { address output.nonnull
      format.date "year" output.check
      editor empty$
	'skip$
	{ organization output }
      if$
      publisher output
    }
  if$
  new.block
  note output
  fin.entry
}

FUNCTION {techreport}
{ output.bibitem
  format.authors "author" output.check
  format.title "title" output.check
  format.tr.number output.nonnull
  institution "institution" output.check
  address output
  format.date "year" output.check
  new.block
  note output
  fin.entry
}

FUNCTION {unpublished}
{ output.bibitem
  format.authors "author" output.check
  format.title "title" output.check
  new.block
  note "note" output.check
  format.date output
  fin.entry
}

FUNCTION {default.type} { misc }

MACRO {acmcs} {"ACM Comput. Surveys"}
MACRO {acta} {"Acta Inf."}
MACRO {cacm} {"Comm. ACM"}
MACRO {ibmjrd} {"IBM J. Res. Dev."}
MACRO {ibmsj} {"IBM Syst.~J."}
MACRO {ieeese} {"IEEE Trans. Softw. Eng."}
MACRO {ieeetc} {"IEEE Trans. Comput."}
MACRO {ieeetcad}
 {"IEEE Trans. Comput.-Aided Design Integrated Circuits"}
MACRO {ipl} {"Inf. Process. Lett."}
MACRO {jacm} {"J.~Assoc. Comput. Mach."}
MACRO {jcss} {"J.~Comput. System Sci."}
MACRO {scp} {"Sci. Comput. Programming"}
MACRO {sicomp} {"SIAM J. Comput."}
MACRO {tocs} {"ACM Trans. Comput. Syst."}
MACRO {tods} {"ACM Trans. Database Syst."}
MACRO {tog} {"ACM Trans. Gr."}
MACRO {toms} {"ACM Trans. Math. Softw."}
MACRO {toois} {"ACM Trans. Office Inf. Syst."}
MACRO {toplas} {"ACM Trans. Prog. Lang. Syst."}
MACRO {tcs} {"Theoretical Comput. Sci."}

READ

FUNCTION {sortify}
{ purify$
  "l" change.case$
}

INTEGERS { len }

FUNCTION {chop.word}
{ 's :=
  'len :=
  s #1 len substring$ =
    { s len #1 + global.max$ substring$ }
    's
  if$
}

FUNCTION {sort.format.names}
{ 's :=
  #1 'nameptr :=
  ""
  s num.names$ 'numnames :=
  numnames 'namesleft :=
    { namesleft #0 > }
    { nameptr #1 >
	{ "   " * }
	'skip$
      if$
      s nameptr "{vv{ } }{ll{ }}{  f{ }}{  jj{ }}" format.name$ 't :=
      nameptr numnames = t "others" = and
	{ fr.et.al sortify * }
	{ t sortify * }
      if$
      nameptr #1 + 'nameptr :=
      namesleft #1 - 'namesleft :=
    }
  while$
}

FUNCTION {sort.format.title}
{ 't :=
  "A " #2
    "An " #3
  "Un " #3
  "Une " #4
  "Des " #4
    "Le " #3
    "La " #3
    "Les " #4
      "The " #4 t chop.word
    chop.word
    chop.word
    chop.word
    chop.word
  chop.word
  chop.word
  chop.word
  chop.word
  sortify
  #1 global.max$ substring$
}

FUNCTION {author.sort}
{ author empty$
    { key empty$
	{ "to sort, need author or key in " cite$ * warning$
	  ""
	}
	{ key sortify }
      if$
    }
    { author sort.format.names }
  if$
}

FUNCTION {author.editor.sort}
{ author empty$
    { editor empty$
	{ key empty$
	    { "to sort, need author, editor, or key in " cite$ * warning$
	      ""
	    }
	    { key sortify }
	  if$
	}
	{ editor sort.format.names }
      if$
    }
    { author sort.format.names }
  if$
}

FUNCTION {author.organization.sort}
{ author empty$
    { organization empty$
	{ key empty$
	    { "to sort, need author, organization, or key in " cite$ * warning$
	      ""
	    }
	    { key sortify }
	  if$
	}
	{ "Le " #3 "La " #3 "Les " #4 
	   "The " #4 organization chop.word 
	  chop.word chop.word chop.word sortify }
      if$
    }
    { author sort.format.names }
  if$
}

FUNCTION {editor.organization.sort}
{ editor empty$
    { organization empty$
	{ key empty$
	    { "to sort, need editor, organization, or key in " cite$ * warning$
	      ""
	    }
	    { key sortify }
	  if$
	}
	{ "Le " #3 "La " #3 "Les " #4 
	   "The " #4 organization chop.word 
	  chop.word chop.word chop.word sortify }
      if$
    }
    { editor sort.format.names }
  if$
}

FUNCTION {presort}
{ type$ "book" =
  type$ "inbook" =
  or
    'author.editor.sort
    { type$ "proceedings" =
	'editor.organization.sort
	{ type$ "manual" =
	    'author.organization.sort
	    'author.sort
	  if$
	}
      if$
    }
  if$
  "    "
  *
  year field.or.null sortify
  *
  "    "
  *
  title field.or.null
  sort.format.title
  *
  #1 entry.max$ substring$
  'sort.key$ :=
}

ITERATE {presort}

SORT

STRINGS { longest.label }

INTEGERS { number.label longest.label.width }

FUNCTION {initialize.longest.label}
{ "" 'longest.label :=
  #1 'number.label :=
  #0 'longest.label.width :=
}

FUNCTION {longest.label.pass}
{ number.label int.to.str$ 'label :=
  number.label #1 + 'number.label :=
  label width$ longest.label.width >
    { label 'longest.label :=
      label width$ 'longest.label.width :=
    }
    'skip$
  if$
}

EXECUTE {initialize.longest.label}

ITERATE {longest.label.pass}

FUNCTION {begin.bib}
{ preamble$ empty$
    'skip$
    { preamble$ write$ newline$ }
  if$
  "\begin{thebibliography}{"  longest.label  * "}" * write$ newline$
  "\expandafter\ifx\csname fonteauteurs\endcsname\relax" 
    write$ newline$
  "\def\fonteauteurs{" fr.auteurs.style * "}\fi" * write$ newline$ 
}

EXECUTE {begin.bib}

EXECUTE {init.state.consts}

EXECUTE {init.last.authors}

ITERATE {call.type$}

FUNCTION {end.bib}
{ newline$
  "\end{thebibliography}" write$ newline$
}

EXECUTE {end.bib}

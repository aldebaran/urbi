%%
%% Bib. style "ieeetr-fr" (version francisee de ieeetr.bst)
%%
%% NM, 2004/03/05
%% markey@lsv.ens-cachan.fr 
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Original Copyright (Oren Patashnik & Howard Trickey):
%%
 % IEEE Transactions bibliography style (29-Jan-88 version)
 %    numeric labels, order-of-reference, IEEE abbreviations,
 %    quotes around article titles, commas separate all fields
 %    except after book titles and before "notes".  Otherwise,
 %    much like the "plain" family, from which this is adapted.
 %
 %   History
 % 9/30/85 (HWT)    Original version, by Howard Trickey.
 % 1/29/88 (OP&HWT) Updated for BibTeX version 0.99a, Oren Patashnik;
 %	THIS `ieeetr' VERSION DOES NOT WORK WITH BIBTEX 0.98i.
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
", \'eds"
}

FUNCTION{fr.og}{		%% "ouvrez les guillemets"
"\og "				%% ATENTION : necessite babel
}

FUNCTION{fr.fg}{		%% "fermez les guillemets"
"\fg{}"				%% ATENTION : necessite babel
}

FUNCTION{fr.et.al}{		%% " et~al."
" \emph{et~al.}"
}

FUNCTION{fr.in}{		%% "/In/ "
"\emph{In} "
}

FUNCTION{fr.in.min}{		%% "/in/ "
"\emph{in} "
}

FUNCTION{fr.in.spc}{		%% " de " (ou " de la s\'erie ")
" de "
}

FUNCTION{fr.of}{		%% " de "
" de "
}

FUNCTION{fr.number}{		%% "num{\'e}ro "
"no.~"
}

FUNCTION{fr.number.maj}{	%% "Num\'ero "
"No.~"
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
"p.~"
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

FUNCTION{fr.deuxpoints}{        %% Pour eviter l'espace avant les ":".
"\string:\penalty500\relax "
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

INTEGERS { output.state before.all mid.sentence after.quote after.sentence
		after.quoted.block after.block after.authors}

FUNCTION {init.state.consts}
{ #0 'before.all :=
  #1 'mid.sentence :=
  #2 'after.quote :=
  #3 'after.sentence :=
  #4 'after.quoted.block :=
  #5 'after.block :=
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
    { output.state after.quote =
	{ " " * write$ }
	{ output.state after.block =
	    { fr.add.period write$
	      newline$
	      "\newblock " write$
	    }
	    { output.state before.all =
		'write$
		{ output.state after.quoted.block =
		    { write$
		      newline$
		      "\newblock " write$
		    }
		    { fr.add.period " " * write$ }
		  if$
		}
	      if$
	    }
	  if$
	}
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

FUNCTION {blank.sep}
{ after.quote 'output.state :=
}

FUNCTION {fin.entry}
{ output.state after.quoted.block =
    'skip$
    'add.period$
  if$
  write$
  newline$
}

FUNCTION {new.block}
{ output.state before.all =
    'skip$
    { output.state after.quote =
	{ after.quoted.block 'output.state := }
	{ after.block 'output.state := }
      if$
    }
  if$
}

FUNCTION {new.sentence}
{ output.state after.block =
    'skip$
    { output.state before.all =
	'skip$
	{ after.sentence 'output.state := }
      if$
    }
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

FUNCTION {new.block.checkb}
{ empty$
  swap$ empty$
  and
    'skip$
    'new.block
  if$
}

FUNCTION {new.sentence.checka}
{ empty$
    'skip$
    'new.sentence
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

INTEGERS { nameptr namesleft numnames }

FUNCTION {format.names}
{ 's :=
  #1 'nameptr :=
  s num.names$ 'numnames :=
  numnames 'namesleft :=
    { namesleft #0 > }
    { s nameptr "{f.~}{vv~}\bgroup\fonteauteurs\bgroup {ll}\egroup\egroup{{}}{, jj}" format.name$ 't :=
      nameptr #1 >
	{ namesleft #1 >
	    { ", " * t * }
	    { %numnames #2 >
		%{ "," * }
		%'skip$
	      %if$
	      t "\bgroup\fonteauteurs\bgroup others\egroup\egroup{}" =
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

FUNCTION {format.authors}
{ author empty$
    { "" }
    { author format.names }
  if$
}

FUNCTION {format.editors}
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
    { fr.og title "t" change.case$ * fr.fg * ", " * }
  if$
}

FUNCTION {format.title.p}
{ title empty$
    { "" }
    { fr.og title "t" change.case$ * fr.fg * ". " * }
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
	{ fr.in.min * series emphasize * }
      if$
      "volume and number" number either.or.check
    }
  if$
}

FUNCTION {format.number.series}
{ volume empty$
    { number empty$
	{ series field.or.null }
	{ output.state mid.sentence =
	    { fr.number }
	    { fr.number.maj }
	  if$
	  number *
	  series empty$
	    { "there's a number but no series in " cite$ * warning$ }
	    { fr.in.spc * series * }
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

FUNCTION {format.volume}
{ volume empty$
    { "" }
    { fr.volume volume * }
  if$
}

FUNCTION {format.number}
{ number empty$
    { "" }
    { fr.number number * }
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
    { fr.in.min booktitle emphasize *
      editor empty$
	'skip$
	{ " (" * format.editors * ")" * }
      if$
    }
  if$
}

FUNCTION {format.thesis.type}
{ type empty$
    'skip$
    { pop$
      output.state after.block =
	{ type "t" change.case$ }
	{ type "l" change.case$ }
      if$
    }
  if$
}

FUNCTION {empty.misc.check}
{ author empty$ title empty$ howpublished empty$
  month empty$ year empty$ note empty$
  and and and and and
    { "all relevant fields are empty in " cite$ * warning$ }
    'skip$
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

FUNCTION {format.addr.pub}
{ publisher empty$
    { "" }
    { address empty$
	{ "" }
	{ address ": " * }
      if$
      publisher *
    }
  if$
}

FUNCTION {format.paddress}
{ address empty$
    { "" }
    { "(" address * ")" * }
  if$
}

FUNCTION {format.article.crossref}
{ key empty$
    { journal empty$
	{ "need key or journal for " cite$ * " to crossref " * crossref *
	  warning$
	  ""
	}
	{ fr.in.min "{\em " * journal * "\/}" * }
      if$
    }
    { fr.in.min key * }
  if$
  " \cite{" * crossref * "}" *
}

FUNCTION {format.crossref.editor}
{ editor #1 "{vv~}\bgroup\fonteauteurs\bgroup {ll}\egroup\egroup{{}}" format.name$
  editor num.names$ duplicate$
  #2 >
    { pop$ fr.et.al * }
    { #2 <
	'skip$
	{ editor #2 "{ff }{vv }{ll}{ jj}" format.name$ "others" =
	    { fr.et.al * }
	    { fr.and  * editor #2 "{vv~}\bgroup\fonteauteurs\bgroup {ll}\egroup\egroup{{}}" format.name$ * }
	  if$
	}
      if$
    }
  if$
}

FUNCTION {format.book.crossref}
{ volume empty$
    { "empty volume in " cite$ * "'s crossref of " * crossref * warning$
      fr.in
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
	    { "{\em " * series * "\/}" * }
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
	    { fr.in.min "{\em " * booktitle * "\/}" * }
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
  blank.sep
  crossref missing$
    { journal emphasize "journal" output.check
      format.volume output
      month empty$
	{ format.number output }
	'skip$
      if$
      format.pages output
      format.date "year" output.check
    }
    { format.article.crossref output.nonnull
      format.pages output
    }
  if$
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
      new.block
      format.number.series output
      format.addr.pub "publisher" output.check
    }
    { new.block
      format.book.crossref output.nonnull
    }
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
  title empty$
    { "empty title in " cite$ * warning$
      howpublished new.sentence.checka
    }
    { howpublished empty$ not
      address empty$ month empty$ year empty$ and and
      or
	{ format.title.p output.nonnull }
	{ format.title output.nonnull }
      if$
      blank.sep
    }
  if$
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
      format.chapter.pages "chapter and pages" output.check
      new.block
      format.number.series output
      format.addr.pub "publisher" output.check
    }
    { format.chapter.pages "chapter and pages" output.check
      new.block
      format.book.crossref output.nonnull
    }
  if$
  format.edition output
  format.date "year" output.check
  new.block
  note output
  fin.entry
}

FUNCTION {incollection}
{ output.bibitem
  format.authors "author" output.check
  format.title "title" output.check
  blank.sep
  crossref missing$
    { format.in.ed.booktitle "booktitle" output.check
      format.bvolume output
      format.number.series output
      format.chapter.pages output
      format.addr.pub "publisher" output.check
      format.edition output
      format.date "year" output.check
    }
    { format.incoll.inproc.crossref output.nonnull
      format.chapter.pages output
    }
  if$
  new.block
  note output
  fin.entry
}

FUNCTION {inproceedings}
{ output.bibitem
  format.authors "author" output.check
  format.title "title" output.check
  blank.sep
  crossref missing$
    { format.in.ed.booktitle "booktitle" output.check
      format.bvolume output
      format.number.series output
      format.paddress output
      format.pages output
      organization output
      publisher output
      format.date "year" output.check
    }
    { format.incoll.inproc.crossref output.nonnull
      format.pages output
    }
  if$
  new.block
  note output
  fin.entry
}

FUNCTION {conference} { inproceedings }

FUNCTION {manual}
{ output.bibitem
  author empty$
    { organization empty$
	'skip$
	{ organization output.nonnull
	  address output
	}
      if$
    }
    { format.authors output.nonnull }
  if$
  format.btitle "title" output.check
  author empty$
    { organization empty$
	{ address new.block.checka
	  address output
	}
	'skip$
      if$
    }
    { organization address new.block.checkb
      organization output
      address output
    }
  if$
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
  blank.sep
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
  title empty$
    { howpublished new.sentence.checka }
    { howpublished empty$ not
      month empty$ year empty$ and
      or
	{ format.title.p output.nonnull }
	{ format.title output.nonnull }
      if$
      blank.sep
    }
  if$
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
  new.block
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
    { organization output }
    { format.editors output.nonnull }
  if$
  format.btitle "title" output.check
  format.bvolume output
  format.number.series output
  format.paddress output
  editor empty$
    'skip$
    { organization output }
  if$
  publisher output
  format.date "year" output.check
  new.block
  note output
  fin.entry
}

FUNCTION {techreport}
{ output.bibitem
  format.authors "author" output.check
  format.title "title" output.check
  blank.sep
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
  format.title.p "title" output.check
  blank.sep
  note "note" output.check
  format.date output
  fin.entry
}

FUNCTION {default.type} { misc }

MACRO {acmcs} {"ACM Computing Surveys"}
MACRO {acta} {"Acta Informatica"}
MACRO {cacm} {"Communications ACM"}
MACRO {ibmjrd} {"IBM J. Research and Development"}
MACRO {ibmsj} {"IBM Systems~J."}
MACRO {ieeese} {"IEEE Trans. Software Engineering"}
MACRO {ieeetc} {"IEEE Trans. Computers"}
MACRO {ieeetcad}
 {"IEEE Trans. Computer-Aided Design"}
MACRO {ipl} {"Information Processing Letters"}
MACRO {jacm} {"J.~ACM"}
MACRO {jcss} {"J.~Computer and System Sciences"}
MACRO {scp} {"Science of Computer Programming"}
MACRO {sicomp} {"SIAM J. Computing"}
MACRO {tocs} {"ACM Trans. Computer Systems"}
MACRO {tods} {"ACM Trans. Database Systems"}
MACRO {tog} {"ACM Trans. Graphics"}
MACRO {toms} {"ACM Trans. Mathematical Software"}
MACRO {toois} {"ACM Trans. Office Information Systems"}
MACRO {toplas} {"ACM Trans. Programming Languages and Systems"}
MACRO {tcs} {"Theoretical Computer Science"}

READ

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

ITERATE {call.type$}

FUNCTION {end.bib}
{ newline$
  "\end{thebibliography}" write$ newline$
}

EXECUTE {end.bib}

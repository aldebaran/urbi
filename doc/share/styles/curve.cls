%%
%% This is file `curve.cls',
%% generated with the docstrip utility.
%%
%% The original source files were:
%%
%% curve.dtx 
%% 


%% Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005 Didier Verna

%% This file is part of CurVe.

%% CurVe may be distributed and/or modified under the
%% conditions of the LaTeX Project Public License, either version 1.1
%% of this license or (at your option) any later version.
%% The latest version of this license is in
%% http://www.latex-project.org/lppl.txt
%% and version 1.1 or later is part of all distributions of LaTeX
%% version 1999/06/01 or later.

%% CurVe consists of the files listed in the file `README'.

%% \CharacterTable
%%  {Upper-case    \A\B\C\D\E\F\G\H\I\J\K\L\M\N\O\P\Q\R\S\T\U\V\W\X\Y\Z
%%   Lower-case    \a\b\c\d\e\f\g\h\i\j\k\l\m\n\o\p\q\r\s\t\u\v\w\x\y\z
%%   Digits        \0\1\2\3\4\5\6\7\8\9
%%   Exclamation   \!     Double quote  \"     Hash (number) \#
%%   Dollar        \$     Percent       \%     Ampersand     \&
%%   Acute accent  \'     Left paren    \(     Right paren   \)
%%   Asterisk      \*     Plus          \+     Comma         \,
%%   Minus         \-     Point         \.     Solidus       \/
%%   Colon         \:     Semicolon     \;     Less than     \<
%%   Equals        \=     Greater than  \>     Question mark \?
%%   Commercial at \@     Left bracket  \[     Backslash     \\
%%   Right bracket \]     Circumflex    \^     Underscore    \_
%%   Grave accent  \`     Left brace    \{     Vertical bar  \|
%%   Right brace   \}     Tilde         \~}
\NeedsTeXFormat{LaTeX2e}

\ProvidesClass{curve}[2005/09/14 v1.10
                      Curriculum Vitae class for LaTeX2e]

\RequirePackage{ltxtable}
\RequirePackage{calc}

\newif\ifstrok\strokfalse
\def\strtest#1#2{%
  \def\@strone{#1}\def\@strtwo{#2}%
  \ifstrok\else\ifx\@strone\@strtwo\stroktrue\fi\fi}

\gdef\@nextentry{}

\def\@keyfont{}
\newcommand\keyfont[1]{\gdef\@keyfont{#1}}

\newcolumntype{k}{>{\@keyfont}l}
\newcommand\keyalignment[1]{%
  \strokfalse\strtest{#1}{l}\strtest{#1}{r}\strtest{#1}{c}%
  \ifstrok\else%
    \ClassError{curve}{Invalid key alignment}{%
      You have called \protect\keyalignment\space with an invalid value.%
      \MessageBreak
      Valid options include l, c, and r.\MessageBreak
      Type X <return> to quit, fix the typo, and rerun LaTeX.}%
  \fi
  \newcolumntype{k}{>{\@keyfont}#1}
}

\def\@prefix{\textbullet}
\newcommand\prefix[1]{\gdef\@prefix{#1}}

\def\@maybekey#1{%
  \def\@newkey{#1}%
  \ifx\@previouskey\@newkey\else%
    #1\gdef\@previouskey{#1}%
  \fi}

\def\@key#1{#1}
\DeclareOption{skipsamekey}{\let\@key\@maybekey}

\newcommand\@entry[2][]{%
  \gdef\@nextentry{}\egroup% end of \noalign opened in \entry.
  \@key{#1}&\@prefix&#2\\}

\newcommand\@sentry[1][]{%
  \gdef\@nextentry{\\}\egroup% end of \noalign opened in \entry.
  \@key{#1}&\@prefix&}

\newcommand\entry{%
  \@nextentry
  \noalign\bgroup\gdef\@beforespace{-\subrubricbeforespace}%
  \@ifstar{\@sentry}{\@entry}}

\def\@subrubricfont{\Large\itshape}
\newcommand\subrubricfont[1]{\gdef\@subrubricfont{#1}}

\newlength\subrubricbeforespace
\setlength\subrubricbeforespace{10pt}

\newlength\subrubricspace
\setlength\subrubricspace{5pt}

\let\@subrubricmark\@gobble

\def\@@subrubric#1{%
  \parbox{0cm}{%
    \raisebox{\@beforespace}{\@subrubricfont#1}%
    \par\vspace\subrubricspace}%
  \@subrubricmark{#1}}


\def\@clcccolsep{\hspace{\tabcolsep}\ifx\@prefix\@empty~\fi}

\def\@subrubric@l#1{\multicolumn{3}{@{}l}{\@@subrubric{#1}}}
\def\@subrubric@c#1{\multicolumn{3}{@{}c}{\@@subrubric{#1}}}
\def\@subrubric@r#1{\multicolumn{3}{@{}r}{\@@subrubric{#1}}}
\def\@subrubric@cl#1{&\multicolumn{2}{@{\@clcccolsep}l}{\@@subrubric{#1}}}
\def\@subrubric@cc#1{&\multicolumn{2}{@{\@clcccolsep}c}{\@@subrubric{#1}}}

\let\@subrubric\@subrubric@cl

\newcommand\subrubricalignment[1]{%
  \def\@curve@temp@a{\let\@subrubric}
  \expandafter\@curve@temp@a\csname @subrubric@#1\endcsname
  \@ifundefined{@subrubric}{%
    \ClassError{curve}{Invalid subrubric alignment}{%
      You have called \protect\subrubricalignment\space with an invalid value.%
      \MessageBreak
      Valid options include l, c, r, cl and cc.\MessageBreak
      Type X <return> to quit, fix the typo, and rerun LaTeX.}}
}

\newcommand\subrubric[1]{%
  \@nextentry
  \noalign{\gdef\@nextentry{}}%
  \@subrubric{#1}\\*}

\newlength{\@almosttextwidth}
\AtBeginDocument{\setlength\@almosttextwidth{\textwidth-\hfuzz}}

\def\@rubricfont{\Large\bfseries}
\newcommand\rubricfont[1]{\gdef\@rubricfont{#1}}

\newlength\rubricspace
\setlength\rubricspace{10pt}

\let\@rubricmark\@gobble

\def\@@rubrichead#1{\@rubricfont#1\par\vspace\rubricspace}

\def\@rubrichead@l#1{\multicolumn{3}{@{}l}{\@@rubrichead{#1}}\\}
\def\@rubrichead@c#1{\multicolumn{3}{@{}c}{\@@rubrichead{#1}}\\}
\def\@rubrichead@r#1{\multicolumn{3}{@{}r}{\@@rubrichead{#1}}\\}
\def\@rubrichead@cl#1{&\multicolumn{2}{@{\@clcccolsep}l}{\@@rubrichead{#1}}\\}
\def\@rubrichead@cc#1{&\multicolumn{2}{@{\@clcccolsep}c}{\@@rubrichead{#1}}\\}

\let\@rubrichead\@rubrichead@c

\newcommand\rubricalignment[1]{%
  \def\@curve@temp@a{\let\@rubrichead}
  \expandafter\@curve@temp@a\csname @rubrichead@#1\endcsname
  \@ifundefined{@rubrichead}{%
    \ClassError{curve}{Invalid rubric alignment}{%
      You have called \protect\rubricalignment\space with an invalid value.%
      \MessageBreak
      Valid options include l, c, r, cl and cc.\MessageBreak
      Type X <return> to quit, fix the typo, and rerun LaTeX.}}
}

\newenvironment{rubric}[1]{%
  %% \begin{rubric}
  \def\raggedright{%
    \@rightskip\@flushglue\rightskip\@rightskip\leftskip\z@skip}%
  \def\raggedleft{%
    \rightskip\z@skip\leftskip\@flushglue\parfillskip\z@skip}%
  \gdef\@beforespace{0pt}%
  \gdef\@nextentry{}%
  \gdef\@previouskey{}
  \begin{longtable}{@{}kl@{~}X}
    \@rubrichead{#1}
    \endfirsthead
    \@rubrichead{#1\@continuedname}
    \endhead
    \noalign{\@rubricmark{#1}}}{%
    %% \end{rubric}
    \@nextentry
  \end{longtable}}

\newcommand\continuedname[1]{\gdef\@continuedname{#1}}

\def\header@scale{.5}
\newcommand\headerscale[1]{\gdef\header@scale{#1}}
\@onlypreamble\headerscale

\newlength\headerspace
\setlength\headerspace{10pt}

\def\@leftheader{%
  \ClassError{curve}{No \protect\leftheader\space given}{%
    You have called \protect\makeheaders, %
    but you didn't provide a left header.\MessageBreak
    Type X <return> to quit, add a call to \protect\lefheader\space %
    in the preamble of your CV,\MessageBreak
    and rerun LaTeX.}}
\newcommand\leftheader[1]{\gdef\@leftheader{#1}}
\@onlypreamble\leftheader

\def\@rightheader{%
  \ClassError{curve}{No \protect\rightheader\space given}{%
    You have called \protect\makeheaders, %
    but you didn't provide a right header.\MessageBreak
    Type X <return> to quit, add a call to \protect\rightheader\space %
    in the preamble of your CV,\MessageBreak
    and rerun LaTeX.}}
\newcommand\rightheader[1]{\gdef\@rightheader{#1}}
\@onlypreamble\rightheader

\def\photo@scale{.1}
\newcommand\photoscale[1]{\gdef\photo@scale{#1}}
\@onlypreamble\photoscale

\newlength\photosep
\setlength\photosep{10pt}

\newcommand\photo[2][l]{%
  \RequirePackage{graphicx}
  \strokfalse\strtest{#1}{l}\strtest{#1}{r}\strtest{#1}{c}%
  \ifstrok\else\ClassError{curve}{Invalid argument to \protect\photo}{%
    Argument 2 of \protect\photo must be `l', `r' or `c'.}\fi
  \def\tmp@cmd{\global\let\makeheaders@}
  \expandafter\tmp@cmd\csname makeheaders@#1\endcsname
  \gdef\photo@file{#2}}
\@onlypreamble\photo

\newlength\photo@width

\def\includephoto@t{%
  \raisebox{.7\baselineskip-\height}{%
    \includegraphics[width=\photo@width]{\photo@file}}}

\def\includephoto@c{%
  \raisebox{-.5\height}{%
    \includegraphics[width=\photo@width]{\photo@file}}}

\def\includephoto@b{\includegraphics[width=\photo@width]{\photo@file}}

\newlength\leftheader@width
\newlength\rightheader@width

\def\makeheaders@l#1{%
  \setlength\photo@width{\photo@scale\textwidth}
  \setlength\leftheader@width{%
    (\textwidth - \photo@width - \photosep) * \real{\header@scale}}
  \setlength\rightheader@width{%
    \textwidth - \photo@width - \photosep - \leftheader@width}
  \parbox[#1]{\photo@width + \photosep}{\includephoto@\hspace\photosep}%
  \parbox[#1]{\leftheader@width}{\@leftheader}%
  \parbox[#1]{\rightheader@width}{\raggedleft\@rightheader}}

\def\makeheaders@c#1{%
  \setlength\photo@width{\photo@scale\textwidth}
  \setlength\leftheader@width{(\textwidth - \photo@width) * \real{.5}}
  \setlength\rightheader@width{\leftheader@width}
  \parbox[#1]{\leftheader@width}{\@leftheader}%
  \parbox[#1]{\photo@width}{\includephoto@}%
  \parbox[#1]{\rightheader@width}{\raggedleft\@rightheader}}

\def\makeheaders@r#1{%
  \setlength\photo@width{\photo@scale\textwidth}
  \setlength\leftheader@width{%
    (\textwidth - \photo@width - \photosep) * \real{\header@scale}}
  \setlength\rightheader@width{%
    \textwidth - \photo@width - \photosep - \leftheader@width}
  \parbox[#1]{\leftheader@width}{\@leftheader}%
  \parbox[#1]{\rightheader@width}{\raggedleft\@rightheader}%
  \parbox[#1]{\photo@width + \photosep}{\hspace\photosep\includephoto@}}

\def\makeheaders@#1{%
  \setlength\leftheader@width{\header@scale\textwidth}%
  \setlength\rightheader@width{\textwidth - \leftheader@width}%
  \parbox[#1]{\leftheader@width}{\@leftheader}%
  \parbox[#1]{\rightheader@width}{\raggedleft\@rightheader}}

\newcommand\makeheaders[1][c]{%
  \strokfalse\strtest{#1}{t}\strtest{#1}{b}\strtest{#1}{c}%
  \ifstrok\else\ClassError{curve}{Invalid argument to \protect\makeheaders}{%
    Argument of \protect\makeheaders must be `t', `b' or `c'.}\fi
  \def\tmp@cmd{\global\let\includephoto@}
  \expandafter\tmp@cmd\csname includephoto@#1\endcsname
  \makeheaders@{#1}%
  \par\vspace\headerspace}

\@onlypreamble\title

\def\@titlefont{\Huge\bfseries}
\newcommand\titlefont[1]{\gdef\@titlefont{#1}}
\@onlypreamble\titlefont

\newlength\titlespace
\setlength\titlespace{0pt}

\let\@subtitle\@undefined
\newcommand\subtitle[1]{\gdef\@subtitle{#1}}
\@onlypreamble\subtitle

\def\@subtitlefont{\huge\itshape}
\newcommand\subtitlefont[1]{\gdef\@subtitlefont{#1}}
\@onlypreamble\subtitlefont

\newcommand\maketitle{%
  \begin{center}
    {\@titlefont\@title}
    \ifx\@subtitle\@undefined\else\\\@subtitlefont\@subtitle\fi
  \end{center}
  \vspace\titlespace}

\let\@flavor\empty
\newcommand\flavor[1]{\gdef\@flavor{#1}
  \ifx\@flavor\empty\else\edef\@flavor{.\@flavor}\fi}

\DeclareOption{ask}{%
  \typein[\@flavor]{Please specify a CV flavor (none by default):}
  \ifx\@flavor\empty\else\edef\@flavor{.\@flavor}\fi}

\def\@curveinput#1{%
  \IfFileExists{#1\@flavor.ltx}{\@iinput{#1\@flavor.ltx}}{%
    \IfFileExists{#1\@flavor.tex}{\@iinput{#1\@flavor.tex}}{%
      \IfFileExists{#1.ltx}{\@iinput{#1.ltx}}{%
        \IfFileExists{#1.tex}{\@iinput{#1.tex}}{%
          \@iinput{#1}}}}}}

\renewcommand\input{\@ifnextchar\bgroup\@curveinput\@@input}

\newcommand\makerubric[1]{\LTXtable{\@almosttextwidth}{#1}}

\let\newblock\par
\newcounter{bibcount}

\newif\if@bibentry
\AtBeginDocument{\@ifpackageloaded{bibentry}{\@bibentrytrue}{}}

\def\bibliography#1{%
  \if@filesw
    \if@bibentry\global\BR@starfalse\fi
    \immediate\write\@auxout{\string\bibdata{#1}}%
  \fi
  \IfFileExists{\jobname.bbl}{\makerubric{\jobname.bbl}}{%
    \typeout{No file \jobname.bbl.}}}

\newcommand\listpubname[1]{\gdef\@listpubname{#1}}

\newenvironment{thebibliography}[1]{%
  \def\@lbibitem[##1]##2{\@sentry[\@biblabel{##1}]%
    \if@filesw{%
      \let\protect\noexpand%
      \immediate\write\@auxout{\string\bibcite{##2}{##1}}}
    \fi%
    \ignorespaces}
  \def\@bibitem##1{\@sentry[\stepcounter{bibcount}\@biblabel{\thebibcount}]%
    \if@filesw%
    \immediate\write\@auxout{\string\bibcite{##1}{\thebibcount}}%
    \fi%
    \ignorespaces}
  \renewcommand\bibitem{%
    \@nextentry
    \noalign\bgroup\gdef\@beforespace{-\subrubricbeforespace}%
    \@ifnextchar[\@lbibitem\@bibitem}
  \begin{rubric}{\@listpubname}
  }{%
  \end{rubric}
}

\DeclareOption{english}{%
  \continuedname{~(continued)}
  \listpubname{List of Publications}}
\DeclareOption{french}{%
  \continuedname{~(suite)}
  \listpubname{Liste des Publications}}
\DeclareOption{francais}{%
  \ExecuteOptions{french}}
\DeclareOption{spanish}{%
  \continuedname{~(contin\'ua)}
  \listpubname{Lista de Publicaciones}}
\DeclareOption{italian}{%
  \continuedname{~(continua)}
  \listpubname{Pubblicazioni}}
\DeclareOption{german}{%
  \continuedname{~(fortgesetzt)}
  \listpubname{Verzeichnis der Ver\"offentlichungen}}
\DeclareOption{ngerman}{%
  \ExecuteOptions{german}}
\DeclareOption{danish}{%
  \continuedname{~(fortsat)}
  \listpubname{Udgivelser}}
\DeclareOption{dutch}{%
  \continuedname{~(vervolg)}
  \listpubname{Publicaties}}
\DeclareOption{portuges}{%
  \continuedname{~(continua\c c\~ao)}
  \listpubname{Publica\c c\~oes}}
\DeclareOption{portuguese}{%
  \ExecuteOptions{portuges}}
\DeclareOption{brazil}{%
  \ExecuteOptions{portuges}}
\DeclareOption{brazilian}{%
  \ExecuteOptions{portuges}}

\DeclareOption{a4paper}{
  \setlength\paperheight{297mm}
  \setlength\paperwidth{210mm}}
\DeclareOption{a5paper}{
  \setlength\paperheight{210mm}
  \setlength\paperwidth{148mm}}
\DeclareOption{b5paper}{
  \setlength\paperheight{250mm}
  \setlength\paperwidth{176mm}}
\DeclareOption{letterpaper}{
  \setlength\paperheight{11in}
  \setlength\paperwidth{8.5in}}
\DeclareOption{legalpaper}{
  \setlength\paperheight{14in}
  \setlength\paperwidth{8.5in}}
\DeclareOption{executivepaper}{
  \setlength\paperheight{10.5in}
  \setlength\paperwidth{7.25in}}
\DeclareOption{landscape}{
  \setlength\@tempdima{\paperheight}
  \setlength\paperheight{\paperwidth}
  \setlength\paperwidth{\@tempdima}}

\DeclareOption{10pt}{\def\@ptsize{0}}
\DeclareOption{11pt}{\def\@ptsize{1}}
\DeclareOption{12pt}{\def\@ptsize{2}}

\DeclareOption{oneside}{\@twosidefalse\@mparswitchfalse}
\DeclareOption{twoside}{\@twosidetrue\@mparswitchtrue}

\DeclareOption{draft}{\setlength\overfullrule{5pt}}
\DeclareOption{final}{%
  \setlength\overfullrule{0pt}
  \setlongtables}

\ExecuteOptions{english,letterpaper,10pt,oneside,final}
\ProcessOptions

\input{size1\@ptsize.clo}
\setlength\parindent{0pt}
\setlength\parskip{0pt}
\setlength\tabcolsep{10pt}
\setlength\arrayrulewidth{.4\p@}
\raggedbottom
\onecolumn
\pagestyle{empty}
\pagenumbering{arabic}

\if@twoside
  \def\ps@headings{%
    \let\@oddfoot\@empty\let\@evenfoot\@empty
    \def\@evenhead{\thepage\hfil\slshape\leftmark}%
    \def\@oddhead{{\slshape\rightmark}\hfil\thepage}%
    \let\@mkboth\markboth
    \def\@rubricmark##1{\markboth{\MakeUppercase{##1}}{}}%
    \def\@subrubricmark##1{\markright{##1}}%
  }
\else
  \def\ps@headings{%
    \let\@oddfoot\@empty
    \def\@oddhead{{\slshape\rightmark}\hfil\thepage}%
    \let\@mkboth\markboth
    \def\@rubricmark##1{\markright{\MakeUppercase{##1}}}%
  }
\fi
\def\ps@myheadings{%
  \let\@oddfoot\@empty\let\@evenfoot\@empty
  \def\@evenhead{\thepage\hfil\slshape\leftmark}%
  \def\@oddhead{{\slshape\rightmark}\hfil\thepage}%
  \let\@mkboth\@gobbletwo
  \let\@rubricmark\@gobble
  \let\@subrubricmark\@gobble
}

\endinput
%%
%% End of file `curve.cls'.

%%% mysig-alternate.cls

%% This program may be distributed and/or modified under the
%% conditions of the LaTeX Project Public License, either version 1.1
%% of this license or (at your option) any later version.
%% The latest version of this license is in
%% http://www.latex-project.org/lppl.txt
%% and version 1.1 or later is part of all distributions of LaTeX
%% version 1999/06/01 or later.

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{mysig-alternate}[2007/01/08 v1.0 Wrapper for SIG-ALTERNATE]

\LoadClass{sig-alternate}

%% A set of packages used for SIGCSE papers.
\newcommand{\SqueezeMetaData}
{
  \makeatletter%
  \def\keywords{\if@twocolumn%
    \vspace{1mm}\noindent%
    {\secfnt Keywords }%
    \else \small%
    \quotation%
    \fi}%
  %
  \def\terms{\if@twocolumn%
    \vspace{1mm}\noindent%
    {\secfnt General Terms }%
    \else \small%
    \quotation%
    \fi}%
  \makeatother%
}

%% Make it possible to squeeze the bibliography.
\newcommand{\BibEntryStyle}{}
\newcommand{\SqueezeBibliography}{
  \renewcommand{\BibEntryStyle}{\small}
}

%% Same as the definition in sig-alternate.cls, but using \BibEntryStyle.
\def\thebibliography#1{%
\ifnum\addauflag=0\addauthorsection\global\addauflag=1\fi
     \section[References]{%    <=== OPTIONAL ARGUMENT ADDED HERE
        {References} % was uppercased but this affects pdf bookmarks (SP/GM October 2004)
          \vskip -9pt % GM July 2000 (for tighter spacing)
         \@mkboth{{\refname}}{{\refname}}%
     }%
     \list{[\arabic{enumi}]}{%
         \settowidth\labelwidth{[#1]}%
         \leftmargin\labelwidth
         \advance\leftmargin\labelsep
         \advance\leftmargin\bibindent
         \parsep=0pt\itemsep=1pt % GM July 2000
         \itemindent -\bibindent
         \listparindent \itemindent
         \usecounter{enumi}
     }%
     \let\newblock\@empty
     \raggedright % GM July 2000
     \sloppy
     \sfcode`\.=1000\relax\BibEntryStyle
}

%% A more compact description environment.
\newenvironment{compactdescription}
{%
  \vspace{-0.4\baselineskip}%
  \begin{description}%
    \addtolength{\itemsep}{-0.6\baselineskip}%
  }{%
  \end{description}%
  \vspace{-0.4\baselineskip}%
}

%% A more compact itemize environment.
\newenvironment{compactitemize}
{%
  \vspace{-0.4\baselineskip}%
  \leftmargini 12.5pt
  \begin{itemize}%
    \addtolength{\itemsep}{-0.6\baselineskip}%
  }{%
  \end{itemize}%
  \vspace{-0.4\baselineskip}%
}


%% mysig-alternate.cls ends here.

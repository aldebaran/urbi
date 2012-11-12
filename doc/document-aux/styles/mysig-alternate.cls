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

% From the ACM Copyright Form instructions:
%
%   TRANSFER OF COPYRIGHT AGREEMENT
%   
%   Copyright to the above work (including without limitation, the
%   right to publish the work in whole or in part in any and all forms
%   of media, now or hereafter known) is hereby transferred to the ACM
%   [...] effective as of the date of this agreement, on the
%   understanding that the work has been accepted for publication by
%   ACM.  (Please note the joint author qualification under ACM
%   Copyright Procedures elsewhere on this form. For the complete ACM
%   Copyright Policy, see
%   http://www.acm.org/pubs/copyright_policy.html)
%   
%   However, each of the Employer/Author(s) retains the following
%   rights:
%   
%     [...]
%   
%     4. The right to post author-prepared versions of the work
%        covered by ACM copyright in a personal collection on their
%        own Home Page and on a publicly accessible server of their
%        employer.  Such posting is limited to noncommercial access
%        and personal use by others, and must include this notice both
%        embedded within the full text file and in the accompanying
%        citation display as well, i.e.:
%   
%          "© ACM, (YEAR). This is the author's version of the work.
%          It is posted here by permission of ACM for your personal
%          use. Not for redistribution. The definitive version was
%          published in PUBLICATION, {VOL#, ISS#, (DATE)}
%          http://doi.acm.org/10.1145/nnnnnn.nnnnnn"
%   
%        (Article DOIs are on their citation pages in the ACM Digital
%        Library.)
%
% Hence the following commands.  Define `\personalcopy' to enable
% them.
\newtoks\articledoi
\def\doi#1{\global\articledoi{#1}}
\ifx\personalcopy\undefined
  % Nothing.
\else
  % Personal copy settings.
  % Requires DOI (Digital Object Identifier) to be defined using \doi.
  \newtoks\copyrightetcpers
  \global\copyrightetcpers={Copyright \the\copyrtyr\ ACM.  This is the
    author's version of the work.  It is posted here by permission of
    ACM for your personal use.  Not for redistribution.  The
    definitive version was published in {\confname{\the\conf}}
    \the\acmcopyr\ \the\confinfo\par
    \url{\the\articledoi}}
  \toappear{\the\copyrightetcpers.}
\fi

%% mysig-alternate.cls ends here.

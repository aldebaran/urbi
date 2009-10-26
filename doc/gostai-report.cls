\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{gostai-report}[2009/10/26 v1.0 Gostai Report]

\ProcessOptions

\LoadClass[openright,twoside,11pt]{book}

% Index.  Must be before loading hyperref.
\RequirePackage{myindex}
\RequirePackage{index}
\makeindex
\RequirePackage{urbi-report}
\RequirePackage{indexing}
\newcommand{\lstindex}[1]{\index{#1@\lstinline{#1}}\lstinline{#1}}
\RequirePackage{revision}

\newcommand{\savesections}
{%
  \let\@partOrig\part%
  \let\@chapterOrig\chapter%
  \let\@sectionOrig\section%
  \let\@subsectionOrig\subsection%
  \let\@subsubsectionOrig\subsubsection%
  \let\@paragraphOrig\paragraph%
  \let\@subparagraphOrig\subparagraph%
}
\newcommand{\restoresections}
{%
  \let\part\@partOrig%
  \let\chapter\@chapterOrig%
  \let\section\@sectionOrig%
  \let\subsection\@subsectionOrig%
  \let\subsubsection\@subsubsectionOrig%
  \let\paragraph\@paragraphOrig%
  \let\subparagraph\@subparagraphOrig%
}
\newcommand{\lowersections}
{%
  \let\part\chapter%
  \let\chapter\section%
  \let\section\subsection%
  \let\subsection\subsubsection%
  \let\subsubsection\paragraph%
  \let\paragraph\subparagraph%
}
\newcommand{\raisesections}
{%
  \let\subparagraph\paragraph%
  \let\paragraph\subsubsection%
  \let\subsubsection\subsection%
  \let\subsection\section%
  \let\section\chapter%
  \let\chapter\part%
}

\endinput

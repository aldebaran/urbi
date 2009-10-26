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
\endinput


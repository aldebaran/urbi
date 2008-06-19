%%% myentcs.cls

%% This program may be distributed and/or modified under the
%% conditions of the LaTeX Project Public License, either version 1.1
%% of this license or (at your option) any later version.
%% The latest version of this license is in
%% http://www.latex-project.org/lppl.txt
%% and version 1.1 or later is part of all distributions of LaTeX
%% version 1999/06/01 or later.

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{myentcs}[2006/12/05 v1.0 Wrapper for ENTCS]

\ifx\IsAFinal\undefined
\PassOptionsToClass{draft}{entcs}
\else
\fi

\LoadClass{entcs}

  \RequirePackage{myhyperref}

%% The fonts they promote are really bad in PDF, let's not use them.
% DV: Tu peux remplacer @ifclasswith par @ifpackagewith.
\@ifclasswith{entcs}{draft}{
  \RequirePackage[draft,inline,nomargin]{fixme}
  \RequirePackage{pxfonts}
  \RequirePackage[numbers]{natbib}
  \newcommand\newblock{\hskip .11em\@plus.33em\@minus.07em}
  \bibliographystyle{abbrvnat}
}{
  %% Don't define \fixme: There should be none!
  \let\citep\cite
  \let\citet\cite
  \bibliographystyle{entcs}
  \renewcommand{\sectionautorefname}{Sect.}
  \renewcommand{\subsectionautorefname}{Sect.}
  \renewcommand{\subsectionautorefname}{Sect.}
  \renewcommand{\subsubsectionautorefname}{Sect.}
  \renewcommand{\figureautorefname}{Fig.}
}

\endinput
%% myentcs.cls ends here.

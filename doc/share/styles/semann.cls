\NeedsTeXFormat{LaTeX2e}

\ProvidesClass{semann}[2003/06/11 v1.1 Seminar Announce Class for LRDE]

\LoadClass[10pt,notitlepage]{article}

\newif\ifpdf
\ifx\pdfoutput\undefined
  \pdffalse
\else
  \pdftrue
\fi

\RequirePackage[a4paper,nohead,nofoot,hmargin=1cm,vmargin=1.5cm]{geometry}

\RequirePackage[french]{mybabel}
\RequirePackage[latin1]{inputenc}
\RequirePackage[T1]{fontenc}
\RequirePackage{palatino}
\RequirePackage{color}

\RequirePackage{amsfonts}

\RequirePackage{graphicx}
\ifpdf
  \graphicspath{{../../common/pdf/}}
\else
  \graphicspath{{../../common/ps/}}
\fi

% Equipment for a two-column page setup.
\RequirePackage{multicol}
\setlength{\columnsep}{20pt}

\RequirePackage{myacronym}
\RequirePackage{mycxx}

\newcommand\lrdelogo{logo_lrde}
\definecolor{title}{rgb}{0.8,0.4,0.1}
\definecolor{author}{rgb}{0.8,0.1,0.4}

\DeclareOption{blue}{
  \renewcommand\lrdelogo{logo_lrde-blue}
  \definecolor{title}{rgb}{.19,.19,.8}
  \definecolor{author}{rgb}{.19,.8,.19}
}
\DeclareOption{logoblue}{\ExecuteOptions{blue}}
\DeclareOption{green}{
  \renewcommand\lrdelogo{logo_lrde-green}
  \definecolor{title}{rgb}{.19,.8,.19}
  \definecolor{author}{rgb}{.8,.19,.19}
}
\DeclareOption{logogreen}{\ExecuteOptions{green}}
\DeclareOption{red}{
  \renewcommand\lrdelogo{logo_lrde-red}
  \definecolor{title}{rgb}{.8,.19,.19}
  \definecolor{author}{rgb}{.19,.19,.8}
}
\DeclareOption{logored}{\ExecuteOptions{red}}
\ProcessOptions

\def\@date#1.#2.#3\relax{%
  \def\@year{20\two@digits{#1}}
  %% #### FIXME: why is the first \space gobbled ?!
  \title{Séminaire du \number#3\space\space
    \ifcase#2%
    \or Janvier\or Février\or Mars\or Avril\or Mai\or Juin\or Juillet%
    \or Août\or Septembre\or Octobre\or Novembre\or Décembre\fi
    \space\@year}}
\renewcommand\date[1]{\expandafter\@date#1\relax}

\newcommand\subtitle[1]{\def\@subtitle{#1}}
\@onlypreamble\subtitle

\def\maketitle{%
  \begin{center}
    {\Large Laboratoire de Recherche et Développement de l'EPITA}
    {\LARGE\color{title}{\bfseries\@title}\\\@subtitle\par}
    \vskip 1.5em
    {\large
      \lineskip .5em
      \begin{tabular}[t]{c}
        \@author
      \end{tabular}\par}
  \end{center}
  \par
  \vspace{1em}
}

\author{\textbf{http://www.lrde.epita.fr}\\
  \small EPITA / LRDE\\
  14-16 rue Voltaire\\
  94276 Le Kremlin-Bicêtre}

\newcommand\presentation[4][]{%
  \def\@test{#1}\def\@@test{}\ifx\@test\@@test\def\arg{}\else\def\arg{#1 : }\fi
  \textbf{{\Large\arg{\color{title}#2}} -- {\color{author}\scshape#3}}\par
  \begin{center}
    \begin{minipage}{.9\textwidth}
      {\small #4}
    \end{minipage}
  \end{center}
}

\newcommand{\projet}[1]{\section*{\color{author}\textsc{#1}}}

\newcommand{\bibpresentation}[2][]{%
  \presentation[#1]{\titre{#2}}{\auteur{#2}}{\resume{#2}}%
}

\AtBeginDocument{%
  \begin{minipage}{.8\textwidth}
    \makebox[0cm][l]{%
      \hspace{.65\textwidth}
      \includegraphics[clip=false,height=.22\textheight]{\lrdelogo}}
    \vspace{-.22\textheight}
    \maketitle
  \end{minipage}
  \thispagestyle{empty}
}

\endinput

%%% Local Variables:
%%% mode: latex
%%% TeX-master: t
%%% End:

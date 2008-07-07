\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{semplan}[2003/06/18 v1.0 Seminar Plan Class for LRDE]

\RequirePackage{color}

% Size class options.
\newcommand{\authorfontsize}{\large}
\newcommand{\titlefontsize}{\Large}
\newcommand{\logosizefactor}{0.22}

\DeclareOption{small}{
  \renewcommand{\authorfontsize}{\normalsize}
  \renewcommand{\titlefontsize}{\large}
  \renewcommand{\logosizefactor}{0.20}
  \PassOptionsToClass{17pt}{foils}
}

% Color class options.
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

\LoadClass[landscape]{foils}

\newif\ifpdf
\ifx\pdfoutput\undefined
  \pdffalse
\else
  \pdftrue
\fi

\RequirePackage[nohead,nofoot,hmargin=1cm,vmargin=1.5cm]{geometry}

\RequirePackage[french]{babel}
\RequirePackage[latin1]{inputenc}
\RequirePackage[T1]{fontenc}
\RequirePackage{palatino}
\RequirePackage{color}

\RequirePackage{graphicx}
\ifpdf
  \graphicspath{{../../common/pdf/}}
\else
  \graphicspath{{../../common/ps/}}
\fi

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

\author{\textbf{http://www.lrde.epita.fr}\\
  \small EPITA / LRDE\\
  14-16 rue Voltaire\\
  94276 Le Kremlin-Bicêtre}

\newcommand\presentation[3][]{%
  \def\@test{#1}\def\@@test{}\ifx\@test\@@test\def\arg{-}\else\def\arg{#1 :}\fi
\item[\arg]{\color{title}#2}{\begin{flushright}\itshape\color{author}#3\end{flushright}}
}

\newcommand\pause[1]{\item[#1 :] Pause}

\newcommand\projet[1]{\item[]{\color{author}\bf#1}}


%\newcommand\presentation[3]{%
%  \item[#1]{\color{title}#2}\\{\itshape\color{author}#3}
%  }

\AtBeginDocument{%
\foilhead{
    {\authorfontsize Laboratoire de Recherche et Développement de l'EPITA\\}
    \vskip .5em
    {\titlefontsize\color{title}{\bfseries\@title}\\\@subtitle\\}
    \includegraphics[clip=false,height=\logosizefactor\textheight]{\lrdelogo}
}

  \thispagestyle{empty}

  \begin{description}
  }

\AtEndDocument{%
  \end{description}
  }

\endinput

%%% Local Variables:
%%% mode: latex
%%% TeX-master: t
%%% End:

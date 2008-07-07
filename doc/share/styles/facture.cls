%%% facture.cls --- Classe LaTeX pour les factures LRDE

%% Copyright (C) 2002, 2006, 2007 Didier Verna.

%% Author:        Didier Verna <didier@lrde.epita.fr>
%% Maintainer:    Didier Verna <didier@lrde.epita.fr>
%% Created:       Tue Sep 24 16:14:34 2002 under XEmacs 21.5 (beta 5)
%% Last Revision: Tue Sep 24 16:14:34 2002

%% This program may be distributed and/or modified under the
%% conditions of the LaTeX Project Public License, either version 1.1
%% of this license or (at your option) any later version.
%% The latest version of this license is in
%% http://www.latex-project.org/lppl.txt
%% and version 1.1 or later is part of all distributions of LaTeX
%% version 1999/06/01 or later.

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{facture}[2006/03/02 v1.1 Bill Class for LRDE]

\def\@header{{\LARGE Association EPITA}}
\def\@footer{%
  {\small
    Association EPITA -- Association Loi 1901 -- SIRET n$^{\mbox{\tiny o}}$
    443 220 223 00012 code APE 803 Z}}
\DeclareOption{epita}{}
\DeclareOption{transfia}{
  \def\@header{{\LARGE TRANSFIA}\\c/o EPITA}
  \def\@footer{%
    {\small
      TRANSFIA -- Association Loi 1901 -- SIRET n$^{\mbox{\tiny o}}$
      401 872 684 00014}}}

\ProcessOptions

\LoadClass[10pt]{letter}
\RequirePackage[a4paper,nohead,nofoot,hmargin=1.5cm,vmargin=1.5cm]{geometry}
\RequirePackage[francais]{babel}
\RequirePackage[latin1]{inputenc}
\RequirePackage[T1]{fontenc}
\RequirePackage{palatino}
\RequirePackage{tabularx}
\RequirePackage{pifont}
\RequirePackage{eurosym}

\def\@numero{%
  \ClassError{Facture}{Il manque le numero de facture (\protect\numero\space)}{%
    Vous devez appeler la commande \protect\numero\space dans le preambule
    du document.\MessageBreak}}
\newcommand\numero[1]{\gdef\@numero{#1}}
\@onlypreamble\numero

\def\@kind{%
  \ClassError{Facture}{Il manque la nature du document ( (\protect\kind\space))}{%
    Vous devez appeler la commande \protect\kind\space dans le preambule
    du document en précisant devis ou facture.\MessageBreak}}
\newcommand\kind[1]{\gdef\@kind{#1}}
\@onlypreamble\kind

\def\@nom{%
  \ClassError{Facture}{Il manque le nom de la societe}{%
    Vous devez appeler la commande \protect\nom\space dans le preambule
    du document.\MessageBreak}}
\newcommand\nom[1]{\gdef\@nom{#1}}
\@onlypreamble\nom

\def\@adresse{%
  \ClassError{Facture}{Il manque l'adresse}{%
    Vous devez appeler la commande \protect\adresse\space dans le preambule
    du document.\MessageBreak}}
\newcommand\adresse[1]{\gdef\@adresse{#1}}
\@onlypreamble\adresse

\def\@responsable{%
  \ClassError{Facture}{Il manque le nom du responsable}{%
    Vous devez appeler la commande \protect\responsable\space dans le preambule
    du document.\MessageBreak}}
\newcommand\responsable[1]{\gdef\@responsable{#1}}
\@onlypreamble\responsable

\def\@telephone{%
  \ClassError{Facture}{Il manque le numero de telephone du responsable}{%
    Vous devez appeler la commande \protect\telephone\space dans le preambule
    du document.\MessageBreak}}
\renewcommand\telephone[1]{\gdef\@telephone{#1}}
\@onlypreamble\telephone

\def\@reference{%
  \ClassError{Facture}{Il manque la reference de la facture}{%
    Vous devez appeler la commande \protect\reference\space dans le preambule
    du document.\MessageBreak}}
\newcommand\reference[1]{\gdef\@reference{#1}}
\@onlypreamble\reference

\newenvironment{liste}{%
  \tabularx{\textwidth}{|c|c|X|c|r|r|}
  \hline
  \textbf{Référence} & \textbf{Quantité} & \textbf{Description} &
  \textbf{Code T.V.A.} & \textbf{P.U. (\euro)} & \textbf{Montant (\euro)}\\
  \hline}{%
  \endtabularx}

%% <ref>, <qté>, <descr>, <code>, <pu>, <montant>
\newcommand\element[6]{#1&#2&#3&#4&#5&#6\\}

\newcommand\espace[1]{&&~\vspace{#1}&&&\\\hline}

\newcommand\itemTotal[2]{%
  \multicolumn{3}{l}{}&\multicolumn{2}{|r|}{#1}&#2\\
  \cline{4-6}}

\newcommand\totalht[1]{%
  \itemTotal{Total H.T.}{#1}}

\newcommand\tva[2]{%
  \itemTotal{T.V.A.: #1}{\gdef\@tva{#2}\@tva}}

\newcommand\totalttc[1]{%
  \itemTotal{\textbf{Total T.T.C.}}{#1}
  \itemTotal{\textbf{Dont T.V.A.}}{\@tva}}

\AtBeginDocument{%
  \begin{minipage}{.5\textwidth}
    \@header\\
    14/16 rue Voltaire\\
    94276 Le Kremlin-Bicêtre CEDEX\\
    France\\
    \\
    Téléphone: 01 53 14 59 22\\
    Télécopie: 01 53 14 59 22
  \end{minipage}
  \hfill
  \begin{minipage}{.385\textwidth}
    {\LARGE{\bfseries \@kind}}\par
    {\@numero}
  \end{minipage}
  \thispagestyle{empty}

  \rule{\textwidth}{1mm}

  \vspace{5mm}

  \begin{minipage}{.6\textwidth}
    \begin{tabular}{@{}lp{\textwidth}@{}}
      Nom:&\@nom\\
      Adresse:&\@adresse\\
      Responsable:&\@responsable\\
      Téléphone:&\@telephone
    \end{tabular}
  \end{minipage}
  \hfill
  \begin{minipage}{.4\textwidth}
    \begin{tabular}{@{}ll@{}}
      Date:&\@date\\
      Réf. Commande:&\@reference
    \end{tabular}
  \end{minipage}

  \vspace{1cm}}

\AtEndDocument{%
  \vfill

  \fbox{\parbox[l][4cm]{.45\textwidth}{%
      \textbf{Paiment:}\\
      \\
      \ding{114} Comptant\\
      \ding{114} Chèque\\
      \ding{114} Carte de crédit\\
      \\
      Nom: \hrulefill\\
      N$^{o}$: \hrulefill\\
      Expire fin: \hrulefill
    }}
  \hfill
  \fbox{\parbox[l][4cm]{.45\textwidth}{\textbf{Cadre réservé à la société:}}}

  \rule{\textwidth}{1mm}

  \@footer}

\endinput

%% # Utilisation de la class 'facture' -- Didier Verna.
%% # Un exemple vaut mieux qu'un long discours.
%%
%% \documentclass[OPTION]{facture} %% Utiliser l'OPTION 'epita' ou 'transfia'.
%%
%% \date{\today}
%%
%% %% Dans tous les champs suivants, utiliser \par pour changer de ligne.
%%
%% \numero{...}               %% Numéro de la facture.
%% \nom{...}                  %% Nom de la société.
%% \adresse{...}              %% Adresse.
%% \responsable{...}          %% Nom du responsable.
%% \telephone{...}            %% Numéro de téléphone du responsable.
%% \reference{...}            %% Référence de la facture.
%%
%% \begin{document}
%%
%% \begin{liste}
%%   %% \element{Référence}{Quantité}{Description}{Code T.V.A.}{P.U.}{Montant}
%%   \element{...}{...}{...}{...}{...}{...}
%%   \element{...}{...}{...}{...}{...}{...}
%%   %% etc etc.
%%
%%   \espace{...}             %% Pour étirer le tableau vers le bas.
%%   \totalht{...}            %% Total hors taxe.
%%   \tva{...}{...}           %% T.V.A. (pourcentage et montant)
%%   \totalttc{...}           %% Total T.T.C.
%% \end{liste}
%%
%% \end{document}

%% facture.cls ends here.

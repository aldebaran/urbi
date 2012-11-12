
* hyperref, float, algorithm
There are interactions between these guys, and using them properly is
a PITA.  I came to the following conclusion:

- float must be loaded first but not used
- then load hypperref
- then use float.

Note that algorithm requires float (and uses), so there is no solution
to load properly float and hyperref only: you must load algorithm.sty,
then hyperref.sty, and finally algorithm.sty (since it uses
float.sty).

As an example, I use the following sequence in some preamble:

   %% Create new floats.
   \RequirePackage{float}

   % Hyperref should always be included last, but for some reason,
   % there is a very nasty interaction betwen Hyperref and float
   % (included by Algorithm).
   \RequirePackage[bookmarks, bookmarksnumbered, bookmarksopen,
     colorlinks, citecolor=blue, linkcolor=blue, plainpages=false,
     hyperindex, backref, breaklinks=true]{myhyperref}

   % For books and reports, use a numbering similar to that of floats:
   % chapter based.
   \@ifclassloaded{report}{\RequirePackage[chapter]{algorithm}}{
     \@ifclassloaded{book}{\RequirePackage[chapter]{algorithm}}{
       \RequirePackage{algorithm}}}
   \renewcommand{\ALG@name}{Algorithme}

   % \newfloat{ type }{ placement }{ ext }[ within ]
   \newfloat{grammar}{tbp}{logr}[chapter]
   \floatname{grammar}{\iflanguage{french}{Grammaire}{Grammar}}
   \defautorefname{grammar} {grammar} {grammaire}

   % We want a single counter for both tables, algorithms, and figures.
   \let\c@table\c@figure
   \let\c@algorithm\c@figure
   \let\c@grammar\c@figure


Local Variables:
mode: outline
End:

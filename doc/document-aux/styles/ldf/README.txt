There are few things to know about listings.

* otherkeywords => keywords
It does not work properly to define only otherkeywords.  Try the
following example:

   \documentclass{article}
   \usepackage{listings}
   \lstdefinelanguage[]{Yacc}{otherkeywords={\%left}}[]
   \begin{document}
   \begin{lstlisting}[language=Yacc]
   %left
   \end{lstlisting}
   \end{document}

it results in:

   ! Undefined control sequence.
   \lst@OutputToken ...lst@CheckMerge {\lst@thestyle
                                                     {\lst@FontAdjust \setbox \...
   l.6 %left
             right Keyword

you need to define at least one keyword.  You can even define one that
will never be recognized, for instance:

   \lstdefinelanguage[]{Yacc}{otherkeywords={\%left},keywords={\%left}}[keywords]

Local Variables:
mode: outline
End:

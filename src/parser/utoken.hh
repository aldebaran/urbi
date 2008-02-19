#ifndef PARSER_UTOKEN_HH
# define PARSER_UTOKEN_HH

/* The scanner is used for two different purposes: "regular": as a
   regular scanner that feeds its parser, and "prescanner" as a
   pre-scanner that tries to find a complete sentence to feed the
   parser with.

   In the prescanner mode, the scanner looks for the complete command,
   i.e., up to the next terminator (";" or ","), taking care of
   tracking braces, and, of course, occurrences of the terminators in
   the strings or comments etc.

   Flex is not meant to embed two sets of actions in a single scanner,
   so we have to play tricks.  yylex has three optional arguments:
   valp, locp, and up.  In prescanner mode, up = 0, and we are valp
   (which is a pointer) to store a size_t: the length of the full
   sentence.

   There is a number of macros to implement the prescanner mode, and
   to skip the rest of the action in that case: they are all prefixed
   with PRE_*. */


/// Scan \a buf, and return the number of bytes to read to get the
/// next top-level "," or ";" (included).  Return 0 if there is none
/// (maybe the buffer is not complete enough).
///
/// On a "parse-error" (braces do not match), return the length up to
/// (including) the invalid brace, so that the parser will raise a
/// parser error on it.
size_t prescan (char *buf);

#endif // ! PARSER_UTOKEN_HH

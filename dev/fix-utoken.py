#! env python

import re
import sys
file = sys.argv[1]
fin = open(file, 'r')
fout = open(sys.argv[2], 'w')
content = fin.readlines()
for l in content:
  l = re.sub(r'<FlexLexer.h>', '"parser/flex-lexer.hh"', l)
  l = re.sub(r'class istream;', '#include <iostream>', l)
  l = re.sub(r'([	 &])(cin|cout|cerr|[io]stream)', r'\\1std::$\\', l)
  fout.write(l)


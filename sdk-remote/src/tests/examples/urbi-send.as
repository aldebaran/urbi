                                                        -*- shell-script -*-
URBI_INIT

me=$as_me
medir=$(absolute "$0").dir
mkcd $medir

xrun "--help"    urbi-send$EXEEXT --help
xrun "--version" urbi-send$EXEEXT --version

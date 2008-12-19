                                                        -*- shell-script -*-
URBI_INIT

me=$as_me
medir=$(absolute "$0").dir
rm -rf $medir
mkdir -p $medir
cd $medir
xrun "--help"    urbi-send$EXEEXT --help
xrun "--version" urbi-send$EXEEXT --version

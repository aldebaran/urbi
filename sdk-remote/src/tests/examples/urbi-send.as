                                                        -*- shell-script -*-
URBI_INIT

me=$as_me
medir=$(echo "$as_me" | sed -e 's/\..*//').dir
rm -rf $medir
mkdir $medir
cd $medir
xrun "--help"    urbi-send --help
xrun "--version" urbi-send --version

#! /bin/sh

# Merge multiple (2) urbisdk tarballs (one debug and one release)
# and run the installer generator.
set -e
files=
verbose=false
vcredist="/mnt/share/tools/vcredist/vcredist_x86-vcxx-2008.exe"
installer="$HOME/.wine/drive_c/Program Files/NSIS/makensis.exe"
installerargs="/NOCD share/installer/installer.nsh"
# Location of installer.nsh, will create a symlink to it if set
instalscriptloc=
templateloc=
urbiconsole=
# Target
output=

verb ()
{
  if $verbose; then echo "$@" >&2 ; fi
}

ifverb ()
{
  if $verbose; then echo $1 ; else echo $2 ; fi
}

usage ()
{
  cat <<EOF
 usage: $0 [options]

Options:
  -i, --installprogram          Name of installer generator program
                                [$installer]
  -a, --installarguments        Arguments for installer
                                [$installargs]
  -s, --installscriptloc        Directory of installer.nsh
                                [$installscriptloc]
  -t, --templateloc             Directory with templates
                                [$templateloc]
  --vcredist                    vcredist binary
                                [$vcredist]
  --urbi-console		urbi-console installer
				[$urbiconsole]
  -d, --debug                   Debug mode
  -v, --verbose                 Verbose mode
  -o, --output                  Output file
EOF
  exit 0
}

while test $# -ne 0
do
  case $1 in
  (-h|--help) usage;;
  (-i|--installprogram) shift; installer=$1 ;;
  (-a|--installarguments) shift; installerargs=$1 ;;
  (-s|--installscriptloc) shift; installscriptloc=$1 ;;
  (-t|--templateloc) shift; templateloc=$1;;
  (--vcredist) shift; vcredist=$1;;
  (--urbi-console) shift; urbiconsole=$1;;
  (-d|--debug) set -x ;;
  (-v|--verbose) verbose=true ;;
  (-o|--output) shift; output=$1 ;;
  (*) files="$files $1" ;;
  esac
  shift
done

if test "$(echo $files |wc -w)" -ne 2; then
  echo "Expected exactly two files."
  exit 1
fi

verb "merging archives $files"
dir=$(mktemp -d)

if test -n "$output"; then
  trap "cd / && rm -rf $dir" 0 1 2 13 15
fi

verb "Will work in $dir"
cd $dir
mkdir temp
mkdir merge
cd temp
for f in $files; do
  verb merging $f
  unzip $(ifverb "" -q ) $f
  basedir=$(echo *)
  if echo $f |grep -q debug; then
    mv $basedir/bin/urbi-launch.exe $basedir/bin/urbi-launch-d.exe
    mv $basedir/bin/urbi.exe $basedir/bin/urbi-d.exe
    sed  -e 's/urbi-launch/urbi-launch-d/g' $basedir/urbi.bat > $basedir/urbi-d.bat
    rm $basedir/urbi.bat
  fi
  cp -a */* ../merge
  #safer that way
  cd ..
  rm -rf temp
  mkdir temp
  cd temp
done
cd ../merge

verb "Copying vcredist from $vcredist"

cp $vcredist ./vcredist-x86.exe
if ! test -z "$installscriptloc" ; then
  verb "Setting up symlink to $installscriptloc"
  ln -s $installscriptloc share/installer
fi

if ! test -z "$urbiconsole" ; then
  verb "Copying urbi-console installer from $urbiconsole"
  cp "$urbiconsole" ./urbi-console-installer.exe
fi

if ! test -z "templateloc" ; then
  verb "Setting up symlink to $templateloc"
  ln -s $templateloc share/templates
fi

verb "running '$installer' $installerargs"
wine "$installer" $installerargs

if test -n "$output"; then
  mv "$dir/merge/gostai-engine-runtime.exe" "$output"
else
  verb "Result in $dir"
fi

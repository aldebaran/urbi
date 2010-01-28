#! /bin/sh

# Merge multiple (2) urbisdk tarballs (one debug and one release)
# and run the installer generator.
set -e
files=
verbose=false
vcredist="/mnt/share/tools/vcredist/vcredist_x86-vc90.exe"
installer="$HOME/.wine/drive_c/Program Files/NSIS/makensis.exe"
installerargs="/NOCD share/installer/installer.nsh"
# Location of installer.nsh, will create a symlink to it if set
instalscriptloc=
templateloc=
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
	--installprogram|-i  	Name of installer generator program ($installer)
	--installarguments|-a 	Arguments for installer ($installargs)
	--installscriptloc|-s   Directory of installer.nsh ($installscriptloc)
	--templateloc|-t	Directory with templates ($templateloc)
	--debug|-d		Debug mode
	--verbose|-v		Verbose mode
EOF
  exit 0
}
while test $# -ne 0
do
  case $1 in
  (--help|-h) usage;;
  (--installprogram|-i) shift; installer=$1 ;;
  (--installarguments|-a) shift; installerargs=$1 ;;
  (--installscriptloc|-s) shift; installscriptloc=$1 ;;
  (--templateloc|-t) shift; templateloc=$1;;
  (--debug|-d) set -x ;;
  (--verbose|-v) verbose=true ;;
  (*) files="$files $1" ;;
  esac
  shift
done
if test "$(echo $files |wc -w)"  -ne 2; then
  echo "Expected exactly two files."
  exit 1
fi

verb "merging archives $files"
dir=$(mktemp -d)
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

if ! test -z "templateloc" ; then
  verb "Setting up symlink to $templateloc"
  ln -s $templateloc share/templates
fi

verb "running '$installer' $installerargs"
verb "Result in $dir"
"$installer" $installerargs

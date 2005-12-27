AC_INIT(liburbi-c++,0.1)
AM_INIT_AUTOMAKE
AC_PROG_CXX
AC_PROG_RANLIB

OPENR_SDK_PATH=/usr/local/OPEN_R_SDK
AC_ARG_ENABLE([openr],
			  [AC_HELP_STRING([--enable-openr=sdk_path], [Turn on OPENR client])],
			  [case "${enableval}" in
			   no) openr=false;;
			   false)  openr=false;;
			   *) openr=true; if test  ${enableval} != "yes" ; then OPENR_SDK_PATH=${enableval}; fi;;
			   esac],
			   [openr=false])

sh -c "cd lib/jpeg-6b && ./configure"
dnl checking if openr sdk is realy there
echo -n "checking for openr SDK... "
if  test $openr != false 
then 
	if ! test -f $OPENR_SDK_PATH/bin/mipsel-linux-c++
	then
		echo -e "no\n ##mipsel c++ compiler not found in $OPENR_SDK_PATH/bin/, disabling openr"
		openr=false
	else
		echo  $OPENR_SDK_PATH
	fi
fi

AM_CONDITIONAL([OPENR], [test $openr = true])
AC_SUBST(OPENR_SDK_PATH)
AC_CONFIG_FILES([Makefile
          doc/Makefile
          lib/Makefile
          src/Makefile
	  src/examples/Makefile
	  src/liburbi/Makefile	
	  src/utils/Makefile
	  src/liburbi/openr/Makefile
	  src/liburbi/openr/Makefile.aibo
	  lib/jpeg-6b/makefile.aibo
])
AC_OUTPUT

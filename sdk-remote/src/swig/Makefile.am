SUBDIRS = . java

# Try to disable -O2 by all means (or liburbiJNI.SWIGURBIStarterJavaUpcast from wont work)
AM_CPPFLAGS = -I$(top_srcdir)/src/liburbi/swig
#AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS) -fpic
AM_CXXFLAGS =
AM_CFLAGS =

CFLAGS=
CXXFLAGS=
CPPFLAGS=

EXTRA_DIST = liburbi.i

## -------------- ##
## Bindings java  ##
## -------------- ##

if BINDING_JAVA

java_src_outdir = java/liburbi/main
java_package = liburbi.main

SWIG_FLAGS =					\
	-c++ -java -Wall			\
	-outdir $(java_src_outdir)		\
	-I$(top_srcdir)/src/liburbi/uobject/	\
	-I$(top_srcdir)/include/		\
	-package $(java_package)

# Invoque Swig and generate: java sources & C++ jni code (liburbi_java.cc)
liburbi_java.cc: liburbi.i
	$(mkdir_p) $(java_src_outdir)
	$(SWIG) $(SWIG_FLAGS) $(SWIG_ARGS) -o $@.tmp $<
	perl -pi -e 's/FillMeInAsSizeCannotBeDeterminedAutomatically/(arg1)->size/g' $@.tmp
	mv $@.tmp $@

lib_LTLIBRARIES = liburbijava.la

liburbijava_la_SOURCES = 			\
	liburbi_java.cc				\
	liburbi_java.cc.h			\
	callbacks-caller.hh			\
	callbacks-caller.cc			\
	call-macros.hh				\
	liburbi_main_UObjectJava.h		\
	liburbi_main_UObjectJava.cc

liburbijava_la_LIBADD = $(top_builddir)/src/liburbi/liburbi.la $(top_builddir)/src/libuobject/libuobject.la
liburbijava_la_CPPFLAGS = -I/usr/lib/jvm/default-java/include

CLEANFILES =					\
	liburbi_java.cc.h			\
	liburbi_java.cc				\
	$(wildcard $(java_src_outdir)/*.java)


if WIN32
liburbijava_la_LDFLAGS = -Wl,--add-stdcall-alias

# Add generation of jvm library dll link file, because in java lib folder
# there is only the static version jvm.lib and libtool complains and doesn't
# want to create a shared library using it.
libjvm.dll.a:
	echo -e "EXPORTS\\nJNI_CreateJavaVM@12\\nJNI_GetDefaultJavaVMInitArgs@4\\nJNI_GetCreatedJavaVMs@12" > jvm.def
	dlltool --input-def jvm.def --kill-at --dllname jvm.dll --output-lib libjvm.dll.a

BUILT_SOURCES = libjvm.dll.a

CLEANFILES += 					\
	libjvm.dll.a				\
	jvm.def

# link with the generated jvm link file
liburbijava_la_LIBADD += -L. -ljvm

endif
liburbijava_la_CXXFLAGS =	-I$$JAVA_HOME/include				\
				-I$$JAVA_HOME/include/$(JAVA_INCLUDE_DIR)	\
				-I$(top_srcdir)/include/			\
				-I$(top_srcdir)/src/liburbi/uobject/		\
				-I$(top_builddir)/include/ \
				$(BOOST_CPPFLAGS)

# Under windows path are set to C:/cygwin... in libdir, which
# then prevent us to install the .la in a place with a DESTDIR
# libtool complains that he want some path ending in "C:/cygwin..."
# So we remote the C:/cygwin
# On other Os we assume paths do not contains C:/cygwin
dirty-fix:
	perl -pi -e "s,libdir='C:/cygwin,libdir=',g" liburbijava.la

install-exec-local: dirty-fix

endif

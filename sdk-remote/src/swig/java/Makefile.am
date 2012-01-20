## Copyright (C) 2010-2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.


# This makefile does several things:
# - call SWIG to parse C++ headers files, and generate Java (.java)
#   sources files, and C++ sources files (this C++ code does the
#   binding between Java code and Urbi SDK)
# - compile the C++ generated code, plus some code I provide in the
#   src folder in order to create the library liburbijava.so
# - then compile the Java code (all the .java) generated plus the java
#   code of the examples and the java code of the tests. This is done
#   by giving all java sources to tobuild_JAVA variable (and automake
#   <<makes the coffee>>...)
# - finally it takes all the java objects (.class files) and create
#   several jars files (one main jar for the liburbijava, and several
#   for example code and test code)

# Since the rules depends from the result of each other, this makefile
# is quite hackish:
# - compilation of java code depends on generation of java sources by
#   swig
# - creation of jar files depends on compilation of java sources
#   (since jars aggregate java objects)

include $(top_srcdir)/build-aux/make/init.mk
include $(top_srcdir)/build-aux/make/libport.mk

# Try to disable -O2 by all means (or
# liburbiJNI.SWIGURBIStarterJavaUpcast from won't work).
override CXXFLAGS:=$(filter-out -O%,$(CXXFLAGS))

EXTRA_DIST +=					\
  swig/boost_unordered_map.i			\
  swig/generated-comments.i			\
  swig/doc-from-header.pl			\
  swig/java.i					\
  swig/udictionary-javacode.i

java_src_outdir = urbi
java_package = urbi

# Pass CPPFLAGS to get things like -I/opt/local/include.  Don't use
# -Wall, which is -Wextra + "ignore warning disabling".
SWIGFLAGS =                                     \
  -c++ -java -Wextra                            \
  -outdir $(java_src_outdir)                    \
  -MD -MF "$(DEPDIR)/$*.Tcc"                    \
  $(subst -isystem ,-I,$(CPPFLAGS))             \
  $(LIBPORT_CPPFLAGS)                           \
  -I$(sdk_remote_srcdir)/src/liburbi/uobject    \
  -I$(sdk_remote_srcdir)/include                \
  -package $(java_package)

if APPLE
SWIGFLAGS += -DAPPLE
endif

# Invoque SWIG and generate Java sources & C++ JNI code
# (liburbi_java.cc).
#
# It would be most useful to remove the output dir (so that there are
# no stray files from previous runs that may confuse us when trying to
# understand the generated code).  But this is the very same directory
# (urbi/) in which we also have compiled urbi/UMain.java and others to
# urbi/UMain.class.  So removing the directory is doomed to failure.
# Unless we generate SWIG files in another directory.  Meanwhile, do
# not remove the directory.
%.cc %/%.java %.cc.h: swig/%.i
#	$(AM_V_GEN)rm -rf $(java_src_outdir)
	$(AM_V_GEN)$(mkdir_p) $(java_src_outdir) $(DEPDIR)
	$(AM_V_at)test -n "$(SWIG)" || { echo "SWIG not defined"; exit 1; }
	$(AM_V_at)$(SWIG) $(SWIGFLAGS) -o $@.tmp $<
	$(AM_V_at)sed -e 's,$@.tmp,$@,'		\
	    <"$(DEPDIR)/$*.Tcc" >"$(DEPDIR)/$*.Pcc"
	$(AM_V_at)rm $(DEPDIR)/$*.Tcc
	$(AM_V_at)mv $@.tmp $@
CLEANFILES += $(DEPDIR)/java.Pcc $(DEPDIR)/java.Tcc java.cc.tmp

-include $(DEPDIR)/java.Pcc


BUILT_SOURCES += java.cc java.cc.h

lib_LTLIBRARIES += liburbijava@LIBSFX@.la

nodist_liburbijava@LIBSFX@_la_SOURCES =		\
  java.cc					\
  java.cc.h

liburbijava@LIBSFX@_la_SOURCES =		\
  src/callbacks-caller.hh			\
  src/callbacks-caller.cc			\
  src/call-macros.hh				\
  src/converter.hh				\
  src/converter.cc				\
  src/urbi_UObject.h				\
  src/urbi_UObject.cc				\
  src/urbi_Log.cc				\
  src/urbi_Log.h
liburbijava@LIBSFX@_la_LIBADD =					\
  $(sdk_remote_builddir)/src/liburbi/liburbi$(LIBSFX).la	\
  $(sdk_remote_builddir)/src/libuobject/libuobject$(LIBSFX).la
liburbijava@LIBSFX@_la_CPPFLAGS =		\
  $(JAVA_CPPFLAGS)				\
  $(BOOST_CPPFLAGS)				\
  $(LIBPORT_CPPFLAGS)				\
  -I$(sdk_remote_srcdir)/include		\
  -I$(sdk_remote_builddir)/include

CLEANFILES +=					\
  java.cc.h					\
  java.cc					\
  $(java_src_outdir)/*.java			\
  $(ALL_OBJECTS)				\
  $(ALL_JARS)

CLEANDIRS += urbi

if WIN32
#liburbijava@LIBSFX@_la_LDFLAGS = -Wl,--add-stdcall-alias
liburbijava@LIBSFX@_la_LDFLAGS = -no-undefined
# link with the generated jvm link file
liburbijava@LIBSFX@_la_LIBADD += $(JAVA_LDFLAGS) $(JAVA_LIBS)
endif

# Under windows path are set to C:/cygwin... in libdir, which
# then prevent us to install the .la in a place with a DESTDIR
# libtool complains that he want some path ending in "C:/cygwin..."
# So we remote the C:/cygwin
# On other Os we assume paths do not contains C:/cygwin
#dirty-fix:
#	perl -pi -e "s,libdir='C:/cygwin,libdir=',g" liburbijava@LIBSFX@.la

#install-exec-local: dirty-fix

javadir = $(datadir)/sdk-remote/java
JAVAROOT = .

## ------------ ##
##   SDK Java   ##
## ------------ ##

liburbijava_SRC =				\
  gnu/bytecode/Access.java			\
  gnu/bytecode/ArrayClassLoader.java		\
  gnu/bytecode/ArrayType.java			\
  gnu/bytecode/AttrContainer.java		\
  gnu/bytecode/Attribute.java			\
  gnu/bytecode/ClassFileInput.java		\
  gnu/bytecode/ClassType.java			\
  gnu/bytecode/ClassTypeWriter.java		\
  gnu/bytecode/CodeAttr.java			\
  gnu/bytecode/ConstantPool.java		\
  gnu/bytecode/ConstantValueAttr.java		\
  gnu/bytecode/CpoolClass.java			\
  gnu/bytecode/CpoolEntry.java			\
  gnu/bytecode/CpoolNameAndType.java		\
  gnu/bytecode/CpoolRef.java			\
  gnu/bytecode/CpoolString.java			\
  gnu/bytecode/CpoolUtf8.java			\
  gnu/bytecode/CpoolValue1.java			\
  gnu/bytecode/CpoolValue2.java			\
  gnu/bytecode/dump.java			\
  gnu/bytecode/ExceptionsAttr.java		\
  gnu/bytecode/Field.java			\
  gnu/bytecode/Filter.java			\
  gnu/bytecode/IfState.java			\
  gnu/bytecode/InnerClassesAttr.java		\
  gnu/bytecode/Label.java			\
  gnu/bytecode/LineNumbersAttr.java		\
  gnu/bytecode/ListCodeSize.java		\
  gnu/bytecode/LocalVarsAttr.java		\
  gnu/bytecode/Location.java			\
  gnu/bytecode/Member.java			\
  gnu/bytecode/Method.java			\
  gnu/bytecode/MiscAttr.java			\
  gnu/bytecode/ObjectType.java			\
  gnu/bytecode/PrimType.java			\
  gnu/bytecode/Scope.java			\
  gnu/bytecode/SourceDebugExtAttr.java		\
  gnu/bytecode/SourceFileAttr.java		\
  gnu/bytecode/SwitchState.java			\
  gnu/bytecode/TryState.java			\
  gnu/bytecode/Type.java			\
  gnu/bytecode/VarEnumerator.java		\
  gnu/bytecode/Variable.java			\
  gnu/bytecode/ZipArchive.java			\
  gnu/bytecode/ZipLoader.java

liburbijava_SRC +=				\
  urbi/ImageComponent.java			\
  urbi/ImageFilter.java				\
  urbi/ImageSampler.java			\
  urbi/ImageUtilities.java			\
  urbi/MedianFilter.java			\
  urbi/SendSoundSampler.java			\
  urbi/SoundAction.java				\
  urbi/SoundSampler.java			\
  urbi/SoundUtilities.java			\
  urbi/UObjectStarter.java			\
  urbi/UObject.java				\
  urbi/UMain.java				\
  urbi/Log.java

liburbijava_SRC += $(wildcard urbi/*.java)

# javac generate objects files (.class) that contains $ in their name,
# so escape them before constructing .jar file.
liburbijava_OBJECTS =						\
  $(foreach file,$(wildcard gnu/bytecode/*.class),'$(file)')	\
  $(foreach file,$(wildcard urbi/*.class),'$(file)')

liburbijava_JAR = lib/liburbijava.jar
liburbijavadir = $(javadir)

# Below this is a HUGE hack so that we construct a .jar containing
# _all_ java objects ie, .class from the normal java sources + .class
# from the generated java sources
gen-liburbi-java:
	$(AM_V_GEN)$(JAR) -cf $(notdir $(liburbijava_JAR)) $(liburbijava_OBJECTS)
	$(AM_V_at)$(mkdir_p) $(dir $(liburbijava_JAR))
	$(AM_V_at)mv -f $(notdir $(liburbijava_JAR)) $(dir $(liburbijava_JAR))

$(liburbijava_JAR): $(liburbijava_SRC) classtobuild.stamp
	$(AM_V_GEN)$(MAKE) -C . gen-liburbi-java


JAR_RULES =

## ------------ ##
##   Examples   ##
## ------------ ##

uobjectjava_SRC =				\
  examples/uobjectjava/Adder.java		\
  examples/uobjectjava/Feature1.java		\
  examples/uobjectjava/Feature2.java		\
  examples/uobjectjava/Feature3.java		\
  examples/uobjectjava/Feature4.java		\
  examples/uobjectjava/Feature5.java		\
  examples/uobjectjava/Colormap.java		\
  examples/uobjectjava/SimpleUObject.java
uobjectjava_JAR = examples/uobjectjava/UObjectJava.jar
JAR_RULES += uobjectjava
$(uobjectjava_JAR): uobjectjava

uobjectjava_withmain_SRC =				\
  examples/uobjectjava_withmain/Main.java		\
  examples/uobjectjava_withmain/SimpleUObject.java
uobjectjava_withmain_JAR = examples/uobjectjava_withmain/UObjectJava_withmain.jar
JAR_RULES += uobjectjava_withmain
$(uobjectjava_withmain_JAR): uobjectjava_withmain


balltrack_SRC =							\
  examples/urbiballtrackinghead/URBIBallTrackingHead.java	\
  examples/urbiballtrackinghead/CallImage.java
balltrack_JAR = examples/urbiballtrackinghead/URBIBallTrackingHead.jar
#JAR_RULES += balltrack
$(balltrack_JAR): balltrack

bandwidth_SRC =					\
  examples/urbibandwidth/URBIBandWidth.java	\
  examples/urbibandwidth/BW.java
bandwidth_JAR = examples/urbibandwidth/URBIBandWidth.jar
#JAR_RULES += bandwidth
$(bandwidth_JAR): bandwidth

ping_SRC =					\
  examples/urbiping/URBIPing.java		\
  examples/urbiping/UPing.java
ping_JAR = examples/urbiping/URBIPing.jar
ping_OBJECTS = $(ping_SRC:.java=.class)
#JAR_RULES += ping
$(ping_JAR): ping

image_SRC =					\
  examples/urbiimage/ShowImage.java		\
  examples/urbiimage/URBIImage.java
image_JAR = examples/urbiimage/URBIImage.jar
#JAR_RULES += image
$(image_JAR): image

mirror_SRC =					\
  examples/urbimirror/Command.java		\
  examples/urbimirror/URBIMirror.java
mirror_JAR = examples/urbimirror/URBIMirror.jar
#JAR_RULES += mirror
$(mirror_JAR): mirror

play_SRC = examples/urbiplay/URBIPlay.java
play_JAR = examples/urbiplay/URBIPlay.jar
#JAR_RULES += play
$(play_JAR): play

record_SRC =					\
  examples/urbirecord/URBIRecord.java		\
  examples/urbirecord/Command.java
record_JAR = examples/urbirecord/URBIRecord.jar
#JAR_RULES += record
$(record_JAR): record

send_SRC = examples/urbisend/URBISend.java
send_JAR = examples/urbisend/URBISend.jar
#JAR_RULES += send
$(send_JAR): send

sound_SRC =					\
  examples/urbisound/CallSound.java		\
  examples/urbisound/URBISound.java
sound_JAR = examples/urbisound/URBISound.jar
#JAR_RULES += sound
$(sound_JAR): sound

talkie_SRC =					\
  examples/urbitalkie/MicroNotify1.java		\
  examples/urbitalkie/MicroNotify2.java		\
  examples/urbitalkie/URBITalkie.java
talkie_JAR = examples/urbitalkie/URBITalkie.jar
#JAR_RULES += talkie
$(talkie_JAR): talkie


## ------------ ##
##    Tests     ##
## ------------ ##

test_all_SRC = tests/all/All.java
test_all_JAR = tests/all/All.jar
JAR_RULES += test_all
$(test_all_JAR): test_all

test_java_SRC = tests/java/Java.java
test_java_JAR = tests/java/Java.jar
JAR_RULES += test_java
$(test_java_JAR): test_java

test_timer_SRC = tests/timer/Timer.java
test_timer_JAR = tests/timer/Timer.jar
JAR_RULES += test_timer
$(test_timer_JAR): test_timer

# Generate all jars files
.SECONDEXPANSION:
$(JAR_RULES): $$(patsubst %.java,%.class,$$($$@_SRC))
	$(AM_V_GEN)$(JAR) -cf $(notdir $($@_JAR)) $+
	$(AM_V_at)mkdir -p $(dir $($@_JAR))
	$(AM_V_at)mv $(notdir $($@_JAR)) $(dir $($@_JAR))


ALL_SOURCES = $(liburbijava_SRC)
ALL_SOURCES += $(foreach name,$(JAR_RULES),$($(name)_SRC))
ALL_OBJECTS = $(ALL_SOURCES:.java=.class)
# The generation of the *.class is perform by an Automake internal
# target that update classtobuild.stamp.
$(ALL_OBJECTS): classtobuild.stamp
ALL_JARS = $(liburbijava_JAR)
ALL_JARS += $(foreach name,$(JAR_RULES),$($(name)_JAR))

## -------------------- ##
## Compile java sources ##
## -------------------- ##

tobuild_JAVA = $(ALL_SOURCES)
tobuilddir = $(javadir)

## -------------------- ##
## Install java sources ##
## -------------------- ##

# Install liburbi java sources and examples sources
nobase_dist_java_DATA = $(ALL_SOURCES)
nobase_nodist_java_DATA = $(ALL_JARS)

# Install examples manifests
# nobase_dist_java_DATA +=
# 	examples/urbiballtrackinghead/manifest
# 	examples/urbibandwidth/manifest
# 	examples/urbiping/manifest
# 	examples/urbiimage/manifest
# 	examples/urbimirror/manifest
# 	examples/urbiplay/manifest
# 	examples/urbirecord/manifest
# 	examples/urbisend/manifest
# 	examples/urbisound/manifest
# 	examples/urbitalkie/manifest

# Install a sample `build.xml' (equivalent of Makefile for `ant') to
# allow the user to recompile liburbijava.jar from the java sources.
# Install also sample Eclipse project files
dist_java_DATA =				\
  sample/.classpath				\
  sample/.project				\
  sample/uobject.classpath			\
  sample/uobject.project			\
  sample/uobjectjava.launch			\
  sample/uobjectjava_withmain.launch

# 	sample/URBIBallTrackingHead.launch
# 	sample/URBIBandWidth.launch
# 	sample/URBIImage.launch
# 	sample/URBIMirror.launch
# 	sample/URBIPing.launch
# 	sample/URBIPlay.launch
# 	sample/URBIRecord.launch
# 	sample/URBISend.launch
# 	sample/URBISound.launch
# 	sample/URBITalkie.launch

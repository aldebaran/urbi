## Copyright (C) 2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.


javadir = $(pkgdatadir)/java
JAVAROOT = .

## ------------ ##
##   SDK Java   ##
## ------------ ##

gnu_bytecode_SOURCES =				\
	gnu/bytecode/Access.java		\
	gnu/bytecode/ArrayClassLoader.java	\
	gnu/bytecode/ArrayType.java		\
	gnu/bytecode/AttrContainer.java		\
	gnu/bytecode/Attribute.java		\
	gnu/bytecode/ClassFileInput.java	\
	gnu/bytecode/ClassType.java		\
	gnu/bytecode/ClassTypeWriter.java	\
	gnu/bytecode/CodeAttr.java		\
	gnu/bytecode/ConstantPool.java		\
	gnu/bytecode/ConstantValueAttr.java	\
	gnu/bytecode/CpoolClass.java		\
	gnu/bytecode/CpoolEntry.java		\
	gnu/bytecode/CpoolNameAndType.java	\
	gnu/bytecode/CpoolRef.java		\
	gnu/bytecode/CpoolString.java		\
	gnu/bytecode/CpoolUtf8.java		\
	gnu/bytecode/CpoolValue1.java		\
	gnu/bytecode/CpoolValue2.java		\
	gnu/bytecode/dump.java			\
	gnu/bytecode/ExceptionsAttr.java	\
	gnu/bytecode/Field.java			\
	gnu/bytecode/Filter.java		\
	gnu/bytecode/IfState.java		\
	gnu/bytecode/InnerClassesAttr.java	\
	gnu/bytecode/Label.java			\
	gnu/bytecode/LineNumbersAttr.java	\
	gnu/bytecode/ListCodeSize.java		\
	gnu/bytecode/LocalVarsAttr.java		\
	gnu/bytecode/Location.java		\
	gnu/bytecode/Member.java		\
	gnu/bytecode/Method.java		\
	gnu/bytecode/MiscAttr.java		\
	gnu/bytecode/ObjectType.java		\
	gnu/bytecode/PrimType.java		\
	gnu/bytecode/Scope.java			\
	gnu/bytecode/SourceDebugExtAttr.java	\
	gnu/bytecode/SourceFileAttr.java	\
	gnu/bytecode/SwitchState.java		\
	gnu/bytecode/TryState.java		\
	gnu/bytecode/Type.java			\
	gnu/bytecode/VarEnumerator.java		\
	gnu/bytecode/Variable.java		\
	gnu/bytecode/ZipArchive.java		\
	gnu/bytecode/ZipLoader.java

liburbi_SOURCES =				\
	urbi/ImageComponent.java		\
	urbi/ImageFilter.java			\
	urbi/ImageSampler.java			\
	urbi/ImageUtilities.java		\
	urbi/MedianFilter.java			\
	urbi/SendSoundSampler.java		\
	urbi/SoundAction.java			\
	urbi/SoundSampler.java			\
	urbi/SoundUtilities.java		\
	urbi/UObjectStarter.java		\
	urbi/UObject.java			\
	urbi/UMain.java				\
	urbi/Log.java


generated_SOURCES =				\
	$(wildcard urbi/generated/*.java)

liburbijava_JAR = lib/liburbijava.jar
liburbijava_SRC = $(gnu_bytecode_SOURCES) $(liburbi_SOURCES) $(generated_SOURCES)
liburbijavadir = $(javadir)
liburbijava_OBJECTS = $(liburbijava_SRC:.java=.class)

$(liburbijava_JAR): $(liburbijava_OBJECTS)
	$(JAR) -cvf $$(basename $@) $(liburbijava_OBJECTS)
	mkdir -p $$(dirname $@)
	mv $$(basename $@) $$(dirname $@)


## ------------ ##
##   Examples   ##
## ------------ ##

uobjectjava_SRC =				\
	examples/uobjectjava/Main.java		\
	examples/uobjectjava/Adder.java		\
	examples/uobjectjava/Feature1.java	\
	examples/uobjectjava/Feature2.java	\
	examples/uobjectjava/Feature3.java	\
	examples/uobjectjava/Feature4.java	\
	examples/uobjectjava/Feature5.java	\
	examples/uobjectjava/Colormap.java	\
	examples/uobjectjava/SimpleUObject.java

uobjectjava_JAR = examples/uobjectjava/UObjectJava.jar
uobjectjava_OBJECTS = $(uobjectjava_SRC:.java=.class)
$(uobjectjava_JAR): $(uobjectjava_OBJECTS)
	$(JAR) -cvf $$(basename $@) $(uobjectjava_OBJECTS)
	mkdir -p $$(dirname $@)
	mv $$(basename $@) $$(dirname $@)

balltrack_SRC =						\
	examples/urbiballtrackinghead/URBIBallTrackingHead.java	\
	examples/urbiballtrackinghead/CallImage.java
balltrack_JAR = examples/urbiballtrackinghead/URBIBallTrackingHead.jar
balltrack_OBJECTS = $(balltrack_SRC:.java=.class)
$(balltrack_JAR): $(balltrack_OBJECTS)
	$(JAR) -cvf $$(basename $@) $(balltrack_OBJECTS)
	mkdir -p $$(dirname $@)
	mv $$(basename $@) $$(dirname $@)

bandwidth_SRC =					\
	examples/urbibandwidth/URBIBandWidth.java	\
	examples/urbibandwidth/BW.java
bandwidth_JAR = examples/urbibandwidth/URBIBandWidth.jar
bandwidth_OBJECTS = $(bandwidth_SRC:.java=.class)
$(bandwidth_JAR): $(bandwidth_OBJECTS)
	$(JAR) -cvf $$(basename $@) $(bandwidth_OBJECTS)
	mkdir -p $$(dirname $@)
	mv $$(basename $@) $$(dirname $@)

ping_SRC =					\
	examples/urbiping/URBIPing.java		\
	examples/urbiping/UPing.java
ping_JAR = examples/urbiping/URBIPing.jar
ping_OBJECTS = $(ping_SRC:.java=.class)
$(ping_JAR): $(ping_OBJECTS)
	$(JAR) -cvf $$(basename $@) $(ping_OBJECTS)
	mkdir -p $$(dirname $@)
	mv $$(basename $@) $$(dirname $@)

image_SRC =					\
	examples/urbiimage/ShowImage.java	\
	examples/urbiimage/URBIImage.java
image_JAR = examples/urbiimage/URBIImage.jar
image_OBJECTS = $(image_SRC:.java=.class)
$(image_JAR): $(image_OBJECTS)
	$(JAR) -cvf $$(basename $@) $(image_OBJECTS)
	mkdir -p $$(dirname $@)
	mv $$(basename $@) $$(dirname $@)

mirror_SRC =				\
	examples/urbimirror/Command.java	\
	examples/urbimirror/URBIMirror.java
mirror_JAR = examples/urbimirror/URBIMirror.jar
mirror_OBJECTS = $(mirror_SRC:.java=.class)
$(mirror_JAR): $(mirror_OBJECTS)
	$(JAR) -cvf $$(basename $@) $(mirror_OBJECTS)
	mkdir -p $$(dirname $@)
	mv $$(basename $@) $$(dirname $@)

play_SRC =					\
	examples/urbiplay/URBIPlay.java
play_JAR = examples/urbiplay/URBIPlay.jar
play_OBJECTS = $(play_SRC:.java=.class)
$(play_JAR): $(play_OBJECTS)
	$(JAR) -cvf $$(basename $@) $(play_OBJECTS)
	mkdir -p $$(dirname $@)
	mv $$(basename $@) $$(dirname $@)

record_SRC =				\
	examples/urbirecord/URBIRecord.java	\
	examples/urbirecord/Command.java
record_JAR = examples/urbirecord/URBIRecord.jar
record_OBJECTS = $(record_SRC:.java=.class)
$(record_JAR): $(record_OBJECTS)
	$(JAR) -cvf $$(basename $@) $(record_OBJECTS)
	mkdir -p $$(dirname $@)
	mv $$(basename $@) $$(dirname $@)

send_SRC =					\
	examples/urbisend/URBISend.java
send_JAR = examples/urbisend/URBISend.jar
send_OBJECTS = $(send_SRC:.java=.class)
$(send_JAR): $(send_OBJECTS)
	$(JAR) -cvf $$(basename $@) $(send_OBJECTS)
	mkdir -p $$(dirname $@)
	mv $$(basename $@) $$(dirname $@)

sound_SRC =					\
	examples/urbisound/CallSound.java	\
	examples/urbisound/URBISound.java
sound_JAR = examples/urbisound/URBISound.jar
sound_OBJECTS = $(sound_SRC:.java=.class)
$(sound_JAR): $(sound_OBJECTS)
	$(JAR) -cvf $$(basename $@) $(sound_OBJECTS)
	mkdir -p $$(dirname $@)
	mv $$(basename $@) $$(dirname $@)

talkie_SRC =				\
	examples/urbitalkie/MicroNotify1.java	\
	examples/urbitalkie/MicroNotify2.java	\
	examples/urbitalkie/URBITalkie.java
talkie_JAR = examples/urbitalkie/URBITalkie.jar
talkie_OBJECTS = $(talkie_SRC:.java=.class)
$(talkie_JAR): $(talkie_OBJECTS)
	$(JAR) -cvf $$(basename $@) $(talkie_OBJECTS)
	mkdir -p $$(dirname $@)
	mv $$(basename $@) $$(dirname $@)


## ------------ ##
##    Tests     ##
## ------------ ##

all_SRC = tests/all/All.java
all_JAR = tests/all/All.jar
all_OBJECTS = $(all_SRC:.java=.class)
$(all_JAR): $(all_OBJECTS)
	$(JAR) -cvf $$(basename $@) $(all_OBJECTS)
	mkdir -p $$(dirname $@)
	mv $$(basename $@) $$(dirname $@)



ALL_SOURCES = 					\
	$(liburbijava_SRC)			\
	$(uobjectjava_SRC) 			\
	$(balltrack_SRC) 			\
	$(bandwidth_SRC) 			\
	$(ping_SRC) 				\
	$(image_SRC) 				\
	$(mirror_SRC) 				\
	$(play_SRC) 				\
	$(record_SRC) 				\
	$(send_SRC) 				\
	$(sound_SRC) 				\
	$(talkie_SRC)				\
	$(all_SRC)

JAR_FILES =					\
	$(liburbijava_JAR) 			\
	$(uobjectjava_JAR)			\
	$(balltrack_JAR) 			\
	$(bandwidth_JAR) 			\
	$(ping_JAR) 				\
	$(image_JAR) 				\
	$(mirror_JAR) 				\
	$(play_JAR) 				\
	$(record_JAR) 				\
	$(send_JAR) 				\
	$(sound_JAR) 				\
	$(talkie_JAR)				\
	$(all_JAR)

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
nobase_nodist_java_DATA = $(JAR_FILES)

# Install examples manifests
nobase_dist_java_DATA +=			\
	examples/test/manifest			\
	examples/uobjectjava/manifest		\
	examples/urbiballtrackinghead/manifest	\
	examples/urbibandwidth/manifest		\
	examples/urbiping/manifest		\
	examples/urbiimage/manifest		\
	examples/urbimirror/manifest		\
	examples/urbiplay/manifest		\
	examples/urbirecord/manifest		\
	examples/urbisend/manifest		\
	examples/urbisound/manifest		\
	examples/urbitalkie/manifest

# Install a sample `build.xml' (equivalent of Makefile
# for `ant') to allow the user to recompile liburbijava.jar
# from the java sources.
# Install also sample Eclipse project files
dist_java_DATA =				\
	sample/build.xml			\
	sample/.classpath			\
	sample/.project				\
	sample/uobject.classpath		\
	sample/uobject.project			\
	sample/uobjectjava.launch		\
	sample/test.launch			\
	sample/URBIBallTrackingHead.launch	\
	sample/URBIBandWidth.launch		\
	sample/URBIImage.launch			\
	sample/URBIMirror.launch		\
	sample/URBIPing.launch			\
	sample/URBIPlay.launch			\
	sample/URBIRecord.launch		\
	sample/URBISend.launch			\
	sample/URBISound.launch			\
	sample/URBITalkie.launch


clean-local:
	$(RM) $(liburbijava_OBJECTS) $(JAR_FILES)

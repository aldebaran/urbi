## Copyright (C) 2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.


javadir = $(pkgdatadir)/java
JAR=jar
JAVAC=javac

## ------------------- ##
## Define java sources ##
## ------------------- ##

gnu_bytecode_SOURCES =				\
	gnu/bytecode/Access.java		\
	gnu/bytecode/ArrayClassLoader.java	\
	gnu/bytecode/ArrayType.java		\
	gnu/bytecode/AttrContainer.java		\
	gnu/bytecode/Attribute.java		\
	gnu/bytecode/ChangeLog			\
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
	urbi/UObjectJava.java			\
	urbi/UMain.java

generated_SOURCES =				\
	$(wildcard urbi/generated/*.java)

testUObject_SOURCES =				\
	examples/uobjectjava/Main.java		\
	examples/uobjectjava/Adder.java		\
	examples/uobjectjava/Feature1.java	\
	examples/uobjectjava/Feature2.java	\
	examples/uobjectjava/Feature3.java	\
	examples/uobjectjava/Feature4.java	\
	examples/uobjectjava/Feature5.java	\
	examples/uobjectjava/Colormap.java	\
	examples/uobjectjava/SimpleUObject.java

test_SOURCES =					\
	examples/test/test.java			\
	examples/test/testcall.java

balltrack_SOURCES =						\
	examples/urbiballtrackinghead/URBIBallTrackingHead.java	\
	examples/urbiballtrackinghead/CallImage.java

bandwidth_SOURCES =					\
	examples/urbibandwidth/URBIBandWidth.java	\
	examples/urbibandwidth/BW.java

ping_SOURCES =					\
	examples/urbiping/URBIPing.java		\
	examples/urbiping/UPing.java

image_SOURCES =					\
	examples/urbiimage/ShowImage.java	\
	examples/urbiimage/URBIImage.java

mirror_SOURCES =				\
	examples/urbimirror/Command.java	\
	examples/urbimirror/URBIMirror.java

play_SOURCES =					\
	examples/urbiplay/URBIPlay.java

record_SOURCES =				\
	examples/urbirecord/URBIRecord.java	\
	examples/urbirecord/Command.java

send_SOURCES =					\
	examples/urbisend/URBISend.java

sound_SOURCES =					\
	examples/urbisound/CallSound.java	\
	examples/urbisound/URBISound.java

talkie_SOURCES =				\
	examples/urbitalkie/MicroNotify1.java	\
	examples/urbitalkie/MicroNotify2.java	\
	examples/urbitalkie/URBITalkie.java


all_SOURCES =					\
	$(gnu_bytecode_SOURCES)			\
	$(liburbi_SOURCES)			\
	$(generated_SOURCES)			\
	$(testUObject_SOURCES)			\
	$(test_SOURCES)				\
	$(balltrack_SOURCES)			\
	$(bandwidth_SOURCES)			\
	$(ping_SOURCES)				\
	$(image_SOURCES)			\
	$(mirror_SOURCES)			\
	$(play_SOURCES)				\
	$(record_SOURCES)			\
	$(send_SOURCES)				\
	$(sound_SOURCES)			\
	$(talkie_SOURCES)



## -------------------- ##
## Compile java sources ##
## -------------------- ##

JAR_FILES =							\
	lib/liburbijava.jar					\
	examples/urbiplay/URBIPlay.jar				\
	examples/urbirecord/URBIRecord.jar			\
	examples/urbisend/URBISend.jar				\
	examples/urbiping/URBIPing.jar				\
	examples/urbimirror/URBIMirror.jar			\
	examples/urbibandwidth/URBIBandWidth.jar		\
	examples/urbisound/URBISound.jar			\
	examples/urbiballtrackinghead/URBIBallTrackingHead.jar	\
	examples/urbiimage/URBIImage.jar			\
	examples/test/test.jar					\
	examples/urbitalkie/URBITalkie.jar


ANT = ant -f build.xml

ant.stamp: ../java.cc build.xml $(all_SOURCES)
	$(mkdir_p) ./lib
	$(ANT)
	touch ant.stamp

# Generation of the liburbi-java.jar using `ant'
$(JAR_FILES): ant.stamp
	@if test -f $@; then :; else		\
          rm -f ant.stamp;			\
          $(MAKE) $(AM_MAKEFLAGS) ant.stamp;	\
        fi


## -------------------- ##
## Install java sources ##
## -------------------- ##

# Install liburbi java sources and examples sources
nobase_dist_java_DATA =				\
	$(all_SOURCES)

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


nobase_nodist_java_DATA =			\
	$(JAR_FILES)

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
	$(ANT) clean

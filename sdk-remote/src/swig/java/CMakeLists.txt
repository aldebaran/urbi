
include(qibuild/swig/java)
find_package(Java)
include(UseJava)

# Where swig will generate its java files
set(SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/urbi")
# Create the directory where swig will generate cxx file from swig/java.i
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/swig)
# Generate the native library from swig files plus C++ code
qi_swig_wrap_java(urbijava swig/java.i
  PACKAGE urbi
  OUTDIR ${SWIG_OUTDIR}
  CPP
  SRC
  src/callbacks-caller.hh
  src/callbacks-caller.cc
  src/call-macros.hh
  src/converter.hh
  src/converter.cc
  src/urbi_UObject.h
  src/urbi_UObject.cc
  src/urbi_Log.cc
  src/urbi_Log.h
  DEPENDS
  qi urbi uobject-remote
)
if(UNIX AND NOT APPLE)
  # Set rpath so that our native lib find uobject remote lib.
  set_target_properties(urbijava PROPERTIES
    INSTALL_RPATH "\$ORIGIN/../lib:\$ORIGIN/../lib/gostai/remote"
  )
endif()
qi_install_target(urbijava)


# Create liburbijava.jar. This jar load automatically the native
# library, so user do not have to mess with it.
set(GNUBYTECODE_SOURCE
  gnu/bytecode/Access.java
  gnu/bytecode/ArrayClassLoader.java
  gnu/bytecode/ArrayType.java
  gnu/bytecode/AttrContainer.java
  gnu/bytecode/Attribute.java
  gnu/bytecode/ClassFileInput.java
  gnu/bytecode/ClassType.java
  gnu/bytecode/ClassTypeWriter.java
  gnu/bytecode/CodeAttr.java
  gnu/bytecode/ConstantPool.java
  gnu/bytecode/ConstantValueAttr.java
  gnu/bytecode/CpoolClass.java
  gnu/bytecode/CpoolEntry.java
  gnu/bytecode/CpoolNameAndType.java
  gnu/bytecode/CpoolRef.java
  gnu/bytecode/CpoolString.java
  gnu/bytecode/CpoolUtf8.java
  gnu/bytecode/CpoolValue1.java
  gnu/bytecode/CpoolValue2.java
  gnu/bytecode/dump.java
  gnu/bytecode/ExceptionsAttr.java
  gnu/bytecode/Field.java
  gnu/bytecode/Filter.java
  gnu/bytecode/IfState.java
  gnu/bytecode/InnerClassesAttr.java
  gnu/bytecode/Label.java
  gnu/bytecode/LineNumbersAttr.java
  gnu/bytecode/ListCodeSize.java
  gnu/bytecode/LocalVarsAttr.java
  gnu/bytecode/Location.java
  gnu/bytecode/Member.java
  gnu/bytecode/Method.java
  gnu/bytecode/MiscAttr.java
  gnu/bytecode/ObjectType.java
  gnu/bytecode/PrimType.java
  gnu/bytecode/Scope.java
  gnu/bytecode/SourceDebugExtAttr.java
  gnu/bytecode/SourceFileAttr.java
  gnu/bytecode/SwitchState.java
  gnu/bytecode/TryState.java
  gnu/bytecode/Type.java
  gnu/bytecode/VarEnumerator.java
  gnu/bytecode/Variable.java
  gnu/bytecode/ZipArchive.java
  gnu/bytecode/ZipLoader.java
)
set(URBIJAVA_SOURCE
  urbi/ImageComponent.java
  urbi/ImageFilter.java
  urbi/ImageSampler.java
  urbi/ImageUtilities.java
  urbi/MedianFilter.java
  urbi/SendSoundSampler.java
  urbi/SoundAction.java
  urbi/SoundSampler.java
  urbi/SoundUtilities.java
  urbi/UObjectStarter.java
  urbi/UObject.java
  urbi/UMain.java
  urbi/Log.java
)

add_jar(liburbijava
  ${GNUBYTECODE_SOURCE}
  ${URBIJAVA_SOURCE}
  ${SWIG_OUTDIR}
)

add_dependencies(liburbijava urbijava)
install_jar(liburbijava share/sdk-remote/java/lib/)

# Distribute the source files of our jar.
qi_install_data(
  ${GNUBYTECODE_SOURCE}
  ${URBIJAVA_SOURCE}
  SUBFOLDER sdk-remote/java KEEP_RELATIVE_PATHS
)
# Install swig generated files by copying swig outdir directory
install(DIRECTORY ${SWIG_OUTDIR} DESTINATION share/sdk-remote/java)

# Tests jars
add_jar(All tests/all/All.java)
add_jar(Java tests/java/Java.java)
add_jar(Timer tests/timer/Timer.java)

install_jar(All share/sdk-remote/java/tests/all)
install_jar(Java share/sdk-remote/java/tests/java)
install_jar(Timer share/sdk-remote/java/tests/timer)

add_dependencies(All urbijava)
add_dependencies(Java urbijava)
add_dependencies(Timer urbijava)

qi_install_data(
  tests/all/All.java
  tests/java/Java.java
  tests/timer/Timer.java
  SUBFOLDER sdk-remote/java KEEP_RELATIVE_PATHS
)


# Example jars
set(UOBJECTJAVA_EXAMPLE_SOURCES
  examples/uobjectjava/Adder.java
  examples/uobjectjava/Feature1.java
  examples/uobjectjava/Feature2.java
  examples/uobjectjava/Feature3.java
  examples/uobjectjava/Feature4.java
  examples/uobjectjava/Feature5.java
  examples/uobjectjava/Colormap.java
  examples/uobjectjava/SimpleUObject.java
)
set(UOBJECTJAVAWITHMAIN_EXAMPLE_SOURCES
  examples/uobjectjava_withmain/Main.java
  examples/uobjectjava_withmain/SimpleUObject.java
)

add_jar(UObjectJava ${UOBJECTJAVA_EXAMPLE_SOURCES})
add_jar(UObjectJava_withmain ${UOBJECTJAVAWITHMAIN_EXAMPLE_SOURCES})

add_dependencies(UObjectJava urbijava)
add_dependencies(UObjectJava_withmain urbijava)

install_jar(UObjectJava share/sdk-remote/java/examples/uobjectjava)
install_jar(UObjectJava_withmain share/sdk-remote/java/examples/uobjectjava_withmain)

qi_install_data(
  ${UOBJECTJAVA_EXAMPLE_SOURCES}
  ${UOBJECTJAVAWITHMAIN_EXAMPLE_SOURCES}
  SUBFOLDER sdk-remote/java KEEP_RELATIVE_PATHS
)


# Distribute sample Eclipse files.
qi_install_data(
  sample/.classpath
  sample/.project
  sample/uobject.classpath
  sample/uobject.project
#  sample/uobjectjava.launch
#  sample/uobjectjava_withmain.launch
  SUBFOLDER sdk-remote/java
)

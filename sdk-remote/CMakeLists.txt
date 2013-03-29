cmake_minimum_required(VERSION 2.8)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions(-DURBI_COMPILATION_MODE_DEBUG)
endif()

qi_create_bin(urbi-launch
  SRC
    src/bin/urbi-launch.cc
    src/liburbi/urbi-launch.cc
    src/liburbi/urbi-root.cc
  DEPENDS port sched serialize
  )
if (NOT APPLE AND NOT WIN32)
  qi_use_lib(urbi-launch DL)
endif()

qi_stage_bin(urbi-launch)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_definitions(-DLIBPORT_LIBSFX="")
add_definitions(-DLIBPORT_LIBDIRNAME="lib")
add_definitions(-D_USE_MATH_DEFINES)

#In case we link to our libjpeg, mark it as static
add_definitions(-DJPEG_INTERNAL)

set(COMMON_SRC
src/libuco/details.cc
src/libuco/local.mk
src/libuco/uevent.cc
src/libuco/uobject-common.cc
src/libuco/uobject-hub-common.cc
src/libuco/uprop.cc
src/libuco/ustarter.cc
src/libuco/utable.cc
src/libuco/uvar-common.cc
src/libuco/version-check.cc
src/libuvalue/exit.cc
src/libuvalue/package-info.cc
src/libuvalue/ubinary.cc
src/libuvalue/uimage.cc
src/libuvalue/ulist.cc
src/libuvalue/usound.cc
src/libuvalue/uvalue-common.cc
)

set(URBI_SRC
src/liburbi/compatibility.hh
src/liburbi/compatibility.hxx
src/liburbi/kernel-version.cc
src/liburbi/uabstractclient.cc
src/liburbi/uclient.cc
src/liburbi/uconversion.cc
src/liburbi/umain.cc
src/liburbi/umessage.cc
src/liburbi/urbi-launch.cc
src/liburbi/urbi-root.cc
src/liburbi/usyncclient.cc
src/liburbi/utag.cc
)




# Look for system JPEG, use it if found, or use our internal version.
find_package(JPEG)
if(JPEG_FOUND)
  message("Using SYSTEM JPEG")
  set(JPEG_INCLUDE_DIRS ${JPEG_INCLUDE_DIR})
  set(JPEG_PACKAGE JPEG)
else()
  message("Using INTERNAL JPEG")
  add_definitions(-DJPEG_INTERNAL)
  set(JPEG_PACKAGE QIJPEG)
endif()

set(UOBJECT_REMOTE_SRC
  src/libuobject/main.cc
  src/libuobject/remote-ucontext-impl.hh
  src/libuobject/ucallbacks.cc
  src/libuobject/uobject.cc
  src/libuobject/utimer-callback.cc
  src/libuobject/uvar.cc
  src/libuco/urbi-main.cc
)

qi_create_lib(uobject-remote
  SUBFOLDER gostai/remote
  SHARED
  DEPENDS port serialize ${JPEG_PACKAGE} BOOST BOOST_REGEX BOOST_DATE_TIME  BOOST_SYSTEM BOOST_THREAD
  SRC ${COMMON_SRC} ${UOBJECT_REMOTE_SRC} ${URBI_SRC}
)
set_target_properties(uobject-remote
  PROPERTIES
     OUTPUT_NAME uobject
     INSTALL_NAME_DIR "@executable_path/../lib/gostai/engine"
     COMPILE_FLAGS -DBUILDING_URBI_SDK)

if (NOT APPLE AND NOT WIN32)
  qi_use_lib(uobject-remote DL)
endif()

qi_stage_lib(uobject-remote)

add_subdirectory(src/examples)
add_subdirectory(include)

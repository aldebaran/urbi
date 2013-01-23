cmake_minimum_required(VERSION 2.8)

qi_create_bin(urbi-launch
  SRC src/bin/urbi-launch.cc src/liburbi/urbi-root.cc
  DEPENDS port sched serialize
  )
if (NOT APPLE AND NOT WIN32)
  qi_use_lib(urbi-launch DL)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_definitions(-DLIBPORT_URBI_UFLOAT_DOUBLE)
add_definitions(-DLIBPORT_LIBSFX="")
add_definitions(-DLIBPORT_LIBDIRNAME="lib")

set(URBI_SRC
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
src/libuvalue/exit.cc
src/libuvalue/package-info.cc
src/libuvalue/ubinary.cc
src/libuvalue/uimage.cc
src/libuvalue/ulist.cc
src/libuvalue/usound.cc
src/libuvalue/uvalue-common.cc
)

qi_create_lib(urbi ${URBI_SRC} DEPENDS port)

# Look for system JPEG, use it if found, or use our internal version.
find_package(JPEG)
if(JPEG_FOUND)
  message("Using SYSTEM JPEG")
  set(JPEG_INCLUDE_DIRS ${JPEG_INCLUDE_DIR})
  qi_use_lib(urbi JPEG)
else()
  message("Using INTERNAL JPEG")
  qi_use_lib(urbi QIJPEG)
endif()

if (NOT APPLE AND NOT WIN32)
 qi_use_lib(urbi DL)
endif()

qi_stage_lib(urbi)

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
  DEPENDS port serialize BOOST BOOST_DATE_TIME  BOOST_SYSTEM BOOST_THREAD
  SRC ${UOBJECT_REMOTE_SRC}
)
set_target_properties(uobject-remote
  PROPERTIES
     OUTPUT_NAME uobject)

add_subdirectory(src/examples)
add_subdirectory(include)

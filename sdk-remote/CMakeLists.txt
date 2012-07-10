cmake_minimum_required(VERSION 2.8)

qi_create_bin(urbi-launch
  SRC bin/urbi-launch.cc sdk-remote/src/liburbi/urbi-root.cc
  )
qi_use_lib(urbi-launch DL)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_definitions(-DLIBPORT_URBI_UFLOAT_DOUBLE)
add_definitions(-DLIBPORT_LIBSFX="")
add_definitions(-DLIBPORT_LIBDIRNAME="lib")

add_subdirectory(include)

set(URBI_SRC
  libuco/details.cc
libuco/local.mk
libuco/uevent.cc
libuco/uobject-common.cc
libuco/uobject-hub-common.cc
libuco/uprop.cc
libuco/urbi-main.cc
libuco/ustarter.cc
libuco/utable.cc
libuco/uvar-common.cc
libuco/version-check.cc
liburbi/compatibility.hh
liburbi/compatibility.hxx
liburbi/kernel-version.cc
liburbi/local.mk
liburbi/uabstractclient.cc
liburbi/uclient.cc
liburbi/uconversion.cc
liburbi/umain.cc
liburbi/umessage.cc
liburbi/urbi-launch.cc
liburbi/urbi-root.cc
liburbi/usyncclient.cc
liburbi/utag.cc
libuvalue/exit.cc
libuvalue/local.mk
libuvalue/package-info.cc
libuvalue/ubinary.cc
libuvalue/uimage.cc
libuvalue/ulist.cc
libuvalue/usound.cc
libuvalue/uvalue-common.cc
)

qi_create_lib(urbi ${URBI_SRC} DEPENDS port)


set(UOBJECT_REMOTE_SRC
  sdk-remote/src/libuobject/main.cc
sdk-remote/src/libuobject/remote-ucontext-impl.hh
sdk-remote/src/libuobject/ucallbacks.cc
sdk-remote/src/libuobject/uobject.cc
sdk-remote/src/libuobject/utimer-callback.cc
sdk-remote/src/libuobject/uvar.cc
sdk-remote/src/libuco/urbi-main.cc
)

qi_create_lib(uobject-remote
  SUBFOLDER gostai/remote
  DEPENDS BOOST BOOST_DATE_TIME  BOOST_SYSTEM BOOST_THREAD
  SRC ${UOBJECT_REMOTE_SRC}
)

set_target_properties(uobject-remote
  PROPERTIES
     OUTPUT_NAME uobject)



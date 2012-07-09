cmake_minimum_required(VERSION 2.8)
project(kernel)

find_package(qibuild)

add_subdirectory(sdk-remote/libport)

# Create a executable named kernel
# with the source file: main.cpp
qi_create_bin(urbi-launch
  SRC sdk-remote/src/bin/urbi-launch.cc sdk-remote/src/liburbi/urbi-root.cc
  )
qi_use_lib(urbi-launch DL)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sdk-remote/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sdk-remote/src)

add_definitions(-DLIBPORT_URBI_UFLOAT_DOUBLE)
add_definitions(-DLIBPORT_LIBSFX="")
add_definitions(-DLIBPORT_LIBDIRNAME="lib")

set(URBI_PUBLIC_HEADERS
  urbi/customuvar.hh
urbi/customuvar.hxx
urbi/details.hh
urbi/exit.hh
urbi/export.hh
urbi/fwd.hh
urbi/input-port.hh
urbi/input-port.hxx
urbi/kernel-version.hh
urbi/package-info.hh
urbi/qt_umain.hh
urbi/socket.hh
urbi/uabstractclient.hh
urbi/uabstractclient.hxx
urbi/ubinary.hh
urbi/ubinary.hxx
urbi/ublend-type.hh
urbi/uclient.hh
urbi/ucontext-factory.hh
urbi/ucontext.hh
urbi/ucontext.hxx
urbi/ucontext-impl.hh
urbi/ucontext-impl.hxx
urbi/uconversion.hh
urbi/uevent.hh
urbi/uevent.hxx
urbi/uexternal.hh
urbi/uimage.hh
urbi/uimage.hxx
urbi/ulist.hh
urbi/ulist.hxx
urbi/umain.hh
urbi/umessage.hh
urbi/umessage.hxx
urbi/uobject.hh
urbi/uobject-hub.hh
urbi/uobject.hxx
urbi/uproperty.hh
urbi/uprop.hh
urbi/urbi-root.hh
urbi/usound.hh
urbi/usound.hxx
urbi/ustarter.hh
urbi/ustarter.hxx
urbi/usyncclient.hh
urbi/utable.hh
urbi/utag.hh
urbi/utimer-callback.hh
urbi/uvalue.hh
urbi/uvalue.hxx
urbi/uvalue-serialize.hh
urbi/uvar.hh
urbi/uvar.hxx
urbi/version-check.hh
)

set(URBI_PRIVATE
  sdk-remote/src/libuco/details.cc
sdk-remote/src/libuco/local.mk
sdk-remote/src/libuco/uevent.cc
sdk-remote/src/libuco/uobject-common.cc
sdk-remote/src/libuco/uobject-hub-common.cc
sdk-remote/src/libuco/uprop.cc
sdk-remote/src/libuco/urbi-main.cc
sdk-remote/src/libuco/ustarter.cc
sdk-remote/src/libuco/utable.cc
sdk-remote/src/libuco/uvar-common.cc
sdk-remote/src/libuco/version-check.cc
sdk-remote/src/liburbi/compatibility.hh
sdk-remote/src/liburbi/compatibility.hxx
sdk-remote/src/liburbi/kernel-version.cc
sdk-remote/src/liburbi/local.mk
sdk-remote/src/liburbi/uabstractclient.cc
sdk-remote/src/liburbi/uclient.cc
sdk-remote/src/liburbi/uconversion.cc
sdk-remote/src/liburbi/umain.cc
sdk-remote/src/liburbi/umessage.cc
sdk-remote/src/liburbi/urbi-launch.cc
sdk-remote/src/liburbi/urbi-root.cc
sdk-remote/src/liburbi/usyncclient.cc
sdk-remote/src/liburbi/utag.cc
sdk-remote/src/libuvalue/exit.cc
sdk-remote/src/libuvalue/local.mk
sdk-remote/src/libuvalue/package-info.cc
sdk-remote/src/libuvalue/ubinary.cc
sdk-remote/src/libuvalue/uimage.cc
sdk-remote/src/libuvalue/ulist.cc
sdk-remote/src/libuvalue/usound.cc
sdk-remote/src/libuvalue/uvalue-common.cc
)

set(URBI_SRCS
  ${URBI_PUBLIC_HEADERS}
 # ${ALCOMMON_DETAIL_HEADERS}
  ${URBI_PRIVATE}
)


qi_create_lib(urbi ${URBI_SRCS})


qi_use_lib(urbi
 port
)

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
# Add a simple test:
enable_testing()
qi_create_test(test_kernel "test.cpp")

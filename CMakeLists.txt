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



# Go for plugin

#./dev/ast-nodes-gen  src/ast/ < src/ast/ast.yml
#./dev/ast-all-gen  src/ast/ < src/ast/ast.yml
#./dev/ast-cloner-gen  src/ast/ < src/ast/ast.yml
#./dev/ast-dot-printer-gen  src/ast/ < src/ast/ast.yml
#./dev/ast-fwd-gen  src/ast/ < src/ast/ast.yml
#./dev/ast-node-mk-gen  src/ast/ < src/ast/ast.yml
#./dev/ast-pretty-printer-gen  src/ast/ < src/ast/ast.yml
#./dev/ast-readme-gen  src/ast/ < src/ast/ast.yml
#./dev/ast-serializer-gen  src/ast/ < src/ast/ast.yml
#./dev/ast-transformer-gen  src/ast/ < src/ast/ast.yml
#./dev/ast-visitor-gen  src/ast/ < src/ast/ast.yml


set(UOBJECT_PLUGIN_SRC
src/ast/all.cc
  src/ast/analyzer.cc
src/ast/analyzer.hh
src/ast/catches-type.cc
src/ast/catches-type.hh
src/ast/cloner.hxx
src/ast/cloner.cc
src/ast/dot-print.cc
src/ast/dot-print.hh
src/ast/dot-printer.cc
src/ast/event-match.cc
src/ast/event-match.hh
src/ast/event-match.hxx
src/ast/exps-type.cc
src/ast/exps-type.hh
src/ast/factory.cc
src/ast/factory.hh
src/ast/flavor.cc
src/ast/flavor.hh
src/ast/flavor.hxx
src/ast/formal.cc
src/ast/formal.hh
src/ast/loc.cc
src/ast/loc.hh
src/ast/local-declarations-type.cc
src/ast/local-declarations-type.hh
src/ast/nary-fwd.hh
src/ast/new-clone.hh
src/ast/new-clone.hxx
src/ast/parametric-ast.cc
src/ast/parametric-ast.hh
src/ast/parametric-ast.hxx
src/ast/pretty-printer.cc
src/ast/print.cc
src/ast/print.hh
src/ast/symbols-type.cc
src/ast/symbols-type.hh
src/ast/transformer.cc
src/ast/visitor.hxx
src/binder/bind.cc
src/binder/bind.hh
src/binder/binder.cc
src/binder/binder.hh
src/binder/binder.hxx
src/binder/fwd.hh
src/console-server.cc
src/eval/action.hh
src/eval/ast.cc
src/eval/ast.hh
src/eval/ast.hxx
src/eval/call.cc
src/eval/call.hh
src/eval/call.hxx
src/eval/exec.hh
src/eval/exec.hxx
src/eval/fwd.hh
src/eval/raise.cc
src/eval/raise.hh
src/eval/send-message.cc
src/eval/send-message.hh
src/eval/send-message.hxx
src/flower/flow.cc
src/flower/flow.hh
src/flower/flower.cc
src/flower/flower.hh
src/kernel/connection-set.cc
src/kernel/connection-set.hh
src/kernel/connection.cc
src/kernel/connection.hh
src/kernel/server-timer.cc
src/kernel/server-timer.hh
src/kernel/uconnection.cc
src/kernel/ughostconnection.cc
src/kernel/ughostconnection.hh
src/kernel/uobject.cc
src/kernel/uobject.hh
src/kernel/userver.cc
src/kernel/utypes.cc
src/kernel/uvalue-cast.cc
src/kernel/uvalue-cast.hh
src/object/barrier.cc
src/object/centralized-slots.cc
src/object/code.cc
src/object/code.hh
src/object/cxx-helper.hh
src/object/cxx-object.cc
src/object/date.cc
src/object/dictionary.cc
src/object/directory.cc
src/object/duration.cc
src/object/event-handler.cc
src/object/event.cc
src/object/executable.cc
src/object/file.cc
src/object/finalizable.cc
src/object/finalizable.hh
src/object/float.cc
src/object/format-info.cc
src/object/format-info.hh
src/object/format-info.hxx
src/object/formatter.cc
src/object/formatter.hh
src/object/global.cc
src/object/hash-slots.hh
src/object/hash-slots.hxx
src/object/hash.cc
src/object/ioservice.cc
src/object/ioservice.hh
src/object/job.cc
src/object/list.cc
src/object/lobby.cc
src/object/local.mk
src/object/location.cc
src/object/matrix.cc
src/object/object-class.cc
src/object/object-class.hh
src/object/object-size.cc
src/object/object.cc
src/object/path.cc
src/object/position.cc
src/object/primitive.cc
src/object/profile.cc
src/object/profile.hh
src/object/register.cc
src/object/root-classes.cc
src/object/root-classes.hh
src/object/semaphore.cc
src/object/semaphore.hh
src/object/server.cc
src/object/server.hh
src/object/slot.cc
src/object/socket.cc
src/object/socket.hh
src/object/sorted-vector-slots.hh
src/object/sorted-vector-slots.hxx
src/object/string.cc
src/object/subscription.cc
src/object/symbols.cc
src/object/system.cc
src/object/system.hh
src/object/tag.cc
src/object/uconnection.cc
src/object/uconnection.hh
src/object/urbi-exception.cc
src/object/uvalue.cc
src/object/uvalue.hh
src/object/vector-slots.hh
src/object/vector-slots.hxx
src/object/vector.cc
src/parser/flex-lexer.hh
src/parser/fwd.hh
src/parser/is-keyword.cc
src/parser/is-keyword.hh
src/parser/metavar-map.hh
src/parser/metavar-map.hxx
src/parser/parse.cc
src/parser/parse.hh
src/parser/parser-impl.cc
src/parser/parser-impl.hh
src/parser/parser-impl.hxx
src/parser/parser-utils.cc
src/parser/parser-utils.hh
src/parser/transform.cc
src/parser/transform.hh
src/parser/uparser.cc
src/parser/uparser.hh
src/parser/utoken.hh
src/parser/utoken.cc
src/parser/ugrammar.cc
src/rewrite/desugarer.cc
src/rewrite/desugarer.hh
src/rewrite/fwd.hh
src/rewrite/pattern-binder.cc
src/rewrite/pattern-binder.hh
src/rewrite/rescoper.cc
src/rewrite/rescoper.hh
src/rewrite/rewrite.cc
src/rewrite/rewrite.hh
src/runner/exception.cc
src/runner/exception.hh
src/runner/job.cc
src/runner/job.hh
src/runner/job.hxx
src/runner/local.mk
src/runner/raise.cc
src/runner/shell.cc
src/runner/shell.hh
src/runner/sneaker.cc
src/runner/sneaker.hh
src/runner/stacks.cc
src/runner/stacks.hh
src/runner/stacks.hxx
src/runner/state.cc
src/runner/state.hh
src/runner/state.hxx
src/urbi/iostream.cc
src/urbi/iostream.hh
src/urbi/sdk.cc
sdk-remote/src/libuco/urbi-main.cc
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include/urbi/parser/)


# For ast/factory.cc
add_definitions(-D__SRCDIR__="")
add_definitions(-DBUILDING_URBI_SDK)
qi_create_lib(uobject-engine
  SUBFOLDER gostai/engine
  DEPENDS BOOST BOOST_SIGNALS BOOST_FILESYSTEM BOOST_DATE_TIME  BOOST_SYSTEM BOOST_THREAD
  SRC ${UOBJECT_PLUGIN_SRC}
)
qi_stage_lib(uobject-engine)


# Add a simple test:
enable_testing()
qi_create_test(test_kernel "test.cpp")
add_subdirectory(src/object/urbi)




if (NOT DEFINED TEST_DIRS)
  set(TEST_DIRS
    0.x
    1.x
    2.x
    demo
    uob/plug
    uob/remote
    uob/rtp
    #uob/java
  )
endif()

set(BLACKLIST
  2_x_mem-check_chk
  2_x_load_load_chk
  2_x_bench_sinus_chk
  2_x_derive_chk
  2_x_tfail-mem-check_chk
  2_x_socket_receive_chk
  2_x_filesystem_directory-events_chk
  2_x_filesystem_directory_chk
  2_x_filesystem_process_chk
  2_x_global-local_chk
  0_x_2-xfail_chk  #test is xfail to test test suite
  2_x_lobby_chk #fail due to new test infrastructure
  uob_plug_issue-3699_chk #output is lost by new test suite
  )

set(tests

 )

FOREACH(dir ${TEST_DIRS})
  FILE(GLOB_RECURSE dirtests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}  ${dir}/*.chk)
  list(APPEND tests ${dirtests})
ENDFOREACH(dir)

FOREACH(infileName ${tests})
  string(REPLACE "." "_" _out ${infileName})
  string(REPLACE "/" "_" _out2 ${_out})
  list(FIND BLACKLIST ${_out2} _blacklisted)
  if(NOT ${_blacklisted} EQUAL -1)
    MESSAGE("Test ${_out2} blacklisted, skipping")
  else()
    # We need to check fast mode at this point
    file(READ ${infileName} _fcontent LIMIT 512)
    #MESSAGE("Got ${_fcontent}")
    string(FIND "${_fcontent}" "//#no-fast" _found)
    set(_fast "")
    if (${_found} EQUAL -1)
    set(_fast "--fast")
    endif()
    # no fast mode for remotes
    if (infileName MATCHES remote OR infileName MATCHES rtp)
      set(_fast "")
    endif()
    MESSAGE("adding  ${infileName} ${_out} ${_out2} ")
    qi_add_test(${_out2} urbi-launch ARGUMENTS -s -- ${_fast} --host 127.0.0.1 --port 0 urbi/run-test.u ${infileName} TIMEOUT 60)
  endif()
ENDFOREACH(infileName)

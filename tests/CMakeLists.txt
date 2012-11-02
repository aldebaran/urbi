FILE(GLOB_RECURSE tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/2.x  2.x/*.chk)

set(BLACKLIST
  mem-check_chk
  load_load_chk
  bench_sinus_chk
  derive_chk
  tfail-mem-check_chk
  socket_receive_chk
  filesystem_directory-events_chk
  filesystem_directory_chk
  filesystem_process_chk
  global-local_chk
  )

FOREACH(infileName ${tests})
  string(REPLACE "." "_" _out ${infileName})
  string(REPLACE "/" "_" _out2 ${_out})
  list(FIND BLACKLIST ${_out2} _blacklisted)
  if(NOT ${_blacklisted} EQUAL -1)
    MESSAGE("Test ${_out2} blacklisted, skipping")
  else()
    # We need to check fast mode at this point
    file(READ 2.x/${infileName} _fcontent LIMIT 512)
    #MESSAGE("Got ${_fcontent}")
    string(FIND "${_fcontent}" "//#no-fast" _found)
    set(_fast "")
    if (${_found} EQUAL -1)
    set(_fast "--fast")
    endif()
    MESSAGE("adding  ${infileName} ${_out} ${_out2} ")
    qi_add_test(${_out2} urbi-launch ARGUMENTS -s -- ${_fast} urbi/run-test.u ${infileName} TIMEOUT 30)
  endif()
ENDFOREACH(infileName)

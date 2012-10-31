FILE(GLOB_RECURSE tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/2.x  2.x/*.chk)
#MESSAGE(found ${tests})

FOREACH(infileName ${tests})
  string(REPLACE "." "_" _out ${infileName})
  string(REPLACE "/" "_" _out2 ${_out})
  MESSAGE("adding  ${infileName} ${_out} ${_out2} ")
  qi_add_test(${_out2} urbi-launch ARGUMENTS -s -- urbi/run-test.u ${infileName})
ENDFOREACH(infileName)

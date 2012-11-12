function(uobject name path)
  qi_create_lib( ${name}
    SHARED
    SUBFOLDER gostai/uobjects/${path}
    DEPENDS uobject
    SRC ${ARGN}
  )
  set_target_properties( ${name}
    PROPERTIES
      PREFIX ""
  )
endfunction()

# A target named all does not fare well
uobject(uobject_all test test/all.uob/all.cc)
set_target_properties(uobject_all
  PROPERTIES
     OUTPUT_NAME all)

uobject(lib-urbi test test/lib-urbi.uob/liburbi.cc)
uobject(remote test test/remote.uob/remote.cc)
uobject(machine test test/machine.uob/machine.cc test/machine.uob/umachine.cc)
uobject(generic test test/generic.uob/generic.cc)
uobject(ultest test test/ultest.uob/ultest.cc)
uobject(uaccess test test/uaccess.uob/uaccess.cc)
uobject(uaccess3 test test/uaccess3.uob/uaccess.cc)
uobject(uchange test test/uchange.uob/uchange.cc)
uobject(timer test  test/timer.uob/timer.cc)
uobject(threaded test test/threaded.uob/threaded.cc)
uobject(subsumption test test/subsumption.uob/subsumption.cc)
uobject(issue-3699 test test/issue-3699.uob/issue-3699.cc)
uobject(fusion urbi urbi/fusion.uob/fusion.cc)

uobject(rtp urbi urbi/rtp.uob/rtp.cc)
qi_use_lib(rtp ORTP)

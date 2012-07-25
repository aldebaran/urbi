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

uobject(all test test/all.uob/all.cc)
uobject(threaded test test/threaded.uob/threaded.cc)
uobject(fusion urbi urbi/fusion.uob/fusion.cc)


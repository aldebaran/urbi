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
uobject(threaded test test/threaded.uob/threaded.cc)
uobject(fusion urbi urbi/fusion.uob/fusion.cc)


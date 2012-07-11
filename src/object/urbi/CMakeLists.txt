function(umodule name)
  qi_create_lib( ${name}
    SHARED
    SUBFOLDER gostai/uobjects/urbi
    DEPENDS uobject
    SRC ${ARGN}
  )
  set_target_properties( ${name}
    PROPERTIES
      PREFIX ""
  )
endfunction()

umodule(logger logger.cc)
umodule(process process.cc)
umodule(regexp regexp.cc)
qi_use_lib(regexp BOOST_REGEX)
umodule(stream stream.cc input-stream.cc output-stream.cc)

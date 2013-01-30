# IGNORED FILE for now, those modules are built in.

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
      COMPILE_FLAGS -DBUILDING_URBI_MODULE
  )
  if (WIN32)
    qi_use_lib(${name} port uobject)
  endif()
endfunction()

umodule(logger logger.cc)
if(NOT WIN32)
  umodule(process process.cc)
endif()
umodule(regexp regexp.cc)
qi_use_lib(regexp BOOST_REGEX)
umodule(stream stream.cc input-stream.cc output-stream.cc)

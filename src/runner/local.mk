dist_libuobject_la_SOURCES +=			\
  runner/at-handler.cc				\
  runner/at-handler.hh				\
  runner/fwd.hh					\
  runner/interpreter.cc				\
  runner/interpreter-apply.cc			\
  runner/interpreter.hh				\
  runner/interpreter.hxx			\
  runner/interpreter-visit.hxx			\
  runner/raise.cc				\
  runner/raise.hh				\
  runner/runner.cc				\
  runner/runner.hh				\
  runner/runner.hxx				\
  runner/shell.cc				\
  runner/shell.hh				\
  runner/sneaker.cc				\
  runner/sneaker.hh				\
  runner/stack-debug.hh				\
  runner/stacks.cc				\
  runner/stacks.hh


sdk2_runnerdir = $(includedir)/runner
sdk2_runner_HEADERS =  				\
  runner/raise.hh

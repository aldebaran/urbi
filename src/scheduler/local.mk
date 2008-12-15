dist_libuobject_la_SOURCES +=			\
  $(coroutines_sources)				\
  scheduler/exception.hh			\
  scheduler/exception.hxx			\
  scheduler/fwd.hh				\
  scheduler/job.cc				\
  scheduler/job.hh				\
  scheduler/job.hxx				\
  scheduler/pthread-coro.cc			\
  scheduler/pthread-coro.hh			\
  scheduler/pthread-coro.hxx			\
  scheduler/scheduler.cc			\
  scheduler/scheduler.hh			\
  scheduler/scheduler.hxx			\
  scheduler/tag.cc				\
  scheduler/tag.hh				\
  scheduler/tag.hxx

coroutines_sources = 				\
  scheduler/coroutine.hh			\
  scheduler/coroutine.hxx
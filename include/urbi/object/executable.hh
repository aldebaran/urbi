#ifndef OBJECT_EXECUTABLE_HH
# define OBJECT_EXECUTABLE_HH

# include <urbi/object/cxx-object.hh>
# include <urbi/export.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Executable: public CxxObject
    {
    public:
      Executable();
      Executable(rExecutable model);
      runner::Job* make_job(rLobby lobby,
                            sched::Scheduler& sched,
                            const objects_type& args,
                            libport::Symbol name);
      virtual rObject operator() (object::objects_type args);
      
      template <typename S>
      rObject operator()(S self);

      template <typename S, typename T1>
      rObject operator()(S self, T1 arg1);

      template <typename S, typename T1, typename T2>
      rObject operator()(S self, T1 arg1, T2 arg2);

      template <typename S, typename T1, typename T2, typename T3>
      rObject operator()(S self, T1 arg1, T2 arg2, T3 arg3);

      template <typename S, typename T1, typename T2, typename T3, typename T4>
      rObject operator()(S self, T1 arg1, T2 arg2, T3 arg3, T4 arg4);

      template <typename S, typename T1, typename T2, typename T3, typename T4, typename T5>
      rObject operator()(S self, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5);

      template <typename S, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
      rObject operator()(S self, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6);

      template <typename S, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
      rObject operator()(S self, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7);

      template <typename S, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
      rObject operator()(S self, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8);


      URBI_CXX_OBJECT(Executable, CxxObject);
    };
  }
}

# include <urbi/object/cxx-conversions.hxx>

namespace urbi
{
  namespace object
  {
  
    template <typename S>
    rObject Executable::operator()(S self)
    {
      objects_type args;
      args << CxxConvert<S>::from(self);
      return operator()(args);
    }

    template <typename S, typename T1>
    rObject Executable::operator()(S self, T1 arg1)
    {
      objects_type args;
      args << CxxConvert<S>::from(self)
           << CxxConvert<T1>::from(arg1);
      return operator()(args);
    }

    template <typename S, typename T1, typename T2>
    rObject Executable::operator()(S self, T1 arg1, T2 arg2)
    {
      objects_type args;
      args << CxxConvert<S>::from(self)
           << CxxConvert<T1>::from(arg1)
           << CxxConvert<T2>::from(arg2);
      return operator()(args);
    }

    template <typename S, typename T1, typename T2, typename T3>
    rObject Executable::operator()(S self, T1 arg1, T2 arg2, T3 arg3)
    {
      objects_type args;
      args << CxxConvert<S>::from(self)
           << CxxConvert<T1>::from(arg1)
           << CxxConvert<T2>::from(arg2)
           << CxxConvert<T3>::from(arg3);
      return operator()(args);
    }

    template <typename S, typename T1, typename T2, typename T3, typename T4>
    rObject Executable::operator()(S self, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
    {
      objects_type args;
      args << CxxConvert<S>::from(self)
           << CxxConvert<T1>::from(arg1)
           << CxxConvert<T2>::from(arg2)
           << CxxConvert<T3>::from(arg3)
           << CxxConvert<T4>::from(arg4);
      return operator()(args);
    }

    template <typename S, typename T1, typename T2, typename T3, typename T4, typename T5>
    rObject Executable::operator()(S self, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
    {
      objects_type args;
      args << CxxConvert<S>::from(self)
           << CxxConvert<T1>::from(arg1)
           << CxxConvert<T2>::from(arg2)
           << CxxConvert<T3>::from(arg3)
           << CxxConvert<T4>::from(arg4)
           << CxxConvert<T5>::from(arg5);
      return operator()(args);
    }

    template <typename S, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    rObject Executable::operator()(S self, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6)
    {
      objects_type args;
      args << CxxConvert<S>::from(self)
           << CxxConvert<T1>::from(arg1)
           << CxxConvert<T2>::from(arg2)
           << CxxConvert<T3>::from(arg3)
           << CxxConvert<T4>::from(arg4)
           << CxxConvert<T5>::from(arg5)
           << CxxConvert<T6>::from(arg6);
      return operator()(args);
    }

    template <typename S, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    rObject Executable::operator()(S self, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7)
    {
      objects_type args;
      args << CxxConvert<S>::from(self)
           << CxxConvert<T1>::from(arg1)
           << CxxConvert<T2>::from(arg2)
           << CxxConvert<T3>::from(arg3)
           << CxxConvert<T4>::from(arg4)
           << CxxConvert<T5>::from(arg5)
           << CxxConvert<T6>::from(arg6)
           << CxxConvert<T7>::from(arg7);
      return operator()(args);
    }

    template <typename S, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    rObject Executable::operator()(S self, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8)
    {
      objects_type args;
      args << CxxConvert<S>::from(self)
           << CxxConvert<T1>::from(arg1)
           << CxxConvert<T2>::from(arg2)
           << CxxConvert<T3>::from(arg3)
           << CxxConvert<T4>::from(arg4)
           << CxxConvert<T5>::from(arg5)
           << CxxConvert<T6>::from(arg6)
           << CxxConvert<T7>::from(arg7)
           << CxxConvert<T8>::from(arg8);
      return operator()(args);
    }

  }
}

#endif

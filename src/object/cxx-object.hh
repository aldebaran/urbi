#ifndef OBJECT_CXX_OBJECT_HH
# define OBJECT_CXX_OBJECT_HH

# include <vector>

# include <object/object.hh>

namespace object
{
  class CxxObject: public Object
  {
    public:
      CxxObject();
      static void initialize(rObject global);
      template<typename T>
      static bool add(const std::string& name);

      template <typename T>
      class Binder
      {
        public:
          template <typename M>
          void operator()(const libport::Symbol& name, M method);

        private:
          friend class CxxObject;
          Binder(rObject tgt);
          rObject tgt_;
      };

    private:
      class Initializer
      {
        public:
          virtual rObject make_class() = 0;
          virtual libport::Symbol name() = 0;
      };

      template <typename T>
      class TypeInitializer: public Initializer
      {
        public:
          TypeInitializer(const std::string& name);
          virtual rObject make_class();
          virtual libport::Symbol name();
        private:
          libport::Symbol name_;
      };
      typedef std::vector<Initializer*> initializers_type;
      static initializers_type& initializers_get();
  };
}

# include <object/cxx-object.hxx>

#endif

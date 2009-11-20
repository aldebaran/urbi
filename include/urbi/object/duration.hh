#ifndef OBJECT_DURATION_HH
# define OBJECT_DURATION_HH

# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
    class Duration: public CxxObject
    {
    /*---------------.
    | Construction.  |
    `---------------*/

    public:
      Duration(time_t seconds = 0);
      Duration(rDuration model);

    /*-----------.
    | Printing.  |
    `-----------*/

    public:
      std::string asString() const;
      std::string asPrintable() const;

    /*--------------.
    | Conversions.  |
    `--------------*/

    public:
      time_t seconds() const;

    /*----------.
    | Details.  |
    `----------*/

    private:
      time_t _seconds;
      URBI_CXX_OBJECT(Duration);
    };
  }
}

#endif

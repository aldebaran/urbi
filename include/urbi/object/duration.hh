#ifndef OBJECT_DURATION_HH
# define OBJECT_DURATION_HH

# include <urbi/object/float.hh>

namespace urbi
{
  namespace object
  {
    class Duration: public Float
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
      float seconds() const;

    /*----------.
    | Details.  |
    `----------*/

    private:
      URBI_CXX_OBJECT(Duration);
    };
  }
}

#endif

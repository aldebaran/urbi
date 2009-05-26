#ifndef LIBUOBJECT_COMPATIBILITY_HH
# define LIBUOBJECT_COMPATIBILITY_HH

namespace urbi
{
  namespace compatibility
  {
    /// Evaluate an expression Exp in such a way that the result *and
    /// the errors* are sent to the tag/channel \a tag.
    ///
    /// Urbi 2 sends errors to the "error" tag, which is a problem
    /// here.  If for instance you send an incorrect message on tag
    /// "Foo", it will return an error on the channel "error", and as
    /// a result, the user, still waiting for "Foo", will never be
    /// told there is an error there.
    ///
    /// The open/close combination around Exp gives the following:
    ///
    /// For Urbi 1:
    ///
    ///    name << Exp,
    ///
    /// For Urbi 2:
    ///
    ///      {
    ///        try
    ///        {
    ///          Channel.new(name) << { Exp }
    ///        }
    ///        catch (var e)
    ///        {
    ///          lobby.send("!!! " + e.asString, name);
    ///        }
    ///      },
    ///
    /// We add bracess to allow statements and neutralize precedence
    /// issues.  We end with a `,', as this is an asynchronous command.
    ///
    /// \param name  the name of the channel
    std::string evaluate_in_channel_open(const std::string& name);
    std::string evaluate_in_channel_close(const std::string& name);

    /// Return the string to emit \a event in k1 or k2.
    std::string emit(const std::string& event);

    /// Return the string to test whether \a exp is void in k1 or k2.
    std::string isvoid(const std::string& exp);
  }
}

# include "compatibility.hxx"

#endif // LIBUOBJECT_COMPATIBILITY_HH

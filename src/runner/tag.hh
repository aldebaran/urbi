#ifndef RUNNER_TAG_HH
# define RUNNER_TAG_HH

# include <set>

namespace runner
{

  class Tag
  {
  public:
    // The new tag is always created parentless.
    Tag ();
    virtual ~Tag ();

    // Tag scheduling operations.
    typedef enum { stop, freeze, unfreeze, pause, resume } operation_type;

    // Execute an operation on the local object.
    virtual void act (operation_type);

    // Propagate an operation on the object and its dependents in the right order.
    // Stopping operations will stop the dependents first while starting operations
    // will start the dependents last.
    virtual void propagate (operation_type);

    // Copy parents from another tag.
    void copy_parents (const Tag&);

  protected:
    // Those operations act in both directions: they also update the parents_
    // field of the dependent while updating the dependents_ field of the
    // parent.
    void register_dependent (Tag&);
    void unregister_dependent (Tag&);

  private:
    typedef std::set<Tag*> tags_type;
    tags_type dependents_;
    tags_type parents_;
  };

} // namespace runner

#endif // RUNNER_TAG_HH

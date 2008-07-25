#include <object/centralized-slots.hh>

namespace object
{
  CentralizedSlots::content_type*
    CentralizedSlots::content_ =
    new CentralizedSlots::content_type();

  CentralizedSlots::loc_index_type&
    CentralizedSlots::loc_index_ =
    CentralizedSlots::content_->get<0>();

  CentralizedSlots::obj_index_type&
    CentralizedSlots::obj_index_ =
    CentralizedSlots::content_->get<1>();
}

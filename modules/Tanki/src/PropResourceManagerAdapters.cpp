# include "MapMaster/Tanki/PropResourceManagerFrontend.hpp"
# include "MapMaster/Tanki/PropResourceManagerRaylibAdapter.hpp"

# include "PropResourceManagerFrontend.inl"

namespace MapMaster::Tanki {

template class PropResourceManagerFrontend <PropResourceManagerRaylibAdapter>;

}  // namespace MapMaster::Tanki

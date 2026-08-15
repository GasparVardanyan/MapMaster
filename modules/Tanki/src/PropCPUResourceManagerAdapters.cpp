# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerRaylibAdapter.hpp"

# include "PropCPUResourceManager.inl"

namespace MapMaster::Tanki {

template class PropCPUResourceManager <PropCPUResourceManagerRaylibAdapter>;

}  // namespace MapMaster::Tanki

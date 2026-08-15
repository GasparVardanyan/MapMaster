# include "MapMaster/Tanki/PropGPUResourceManager.hpp"
# include "MapMaster/Tanki/PropGPUResourceManagerRaylibAdapter.hpp"

# include "PropGPUResourceManager.inl"

namespace MapMaster::Tanki {

template class PropGPUResourceManager <PropGPUResourceManagerRaylibAdapter>;

}  // namespace MapMaster::Tanki

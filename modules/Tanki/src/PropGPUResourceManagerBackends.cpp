# include "MapMaster/Tanki/PropGPUResourceManager.hpp"
# include "MapMaster/Tanki/PropGPUResourceManagerRaylibBackend.hpp"

# include "PropGPUResourceManager.inl"

namespace MapMaster::Tanki {

template class PropGPUResourceManager <PropGPUResourceManagerRaylibBackend>;

}  // namespace MapMaster::Tanki

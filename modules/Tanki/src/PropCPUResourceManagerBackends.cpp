# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerRaylibBackend.hpp"

# include "PropCPUResourceManager.inl"

namespace MapMaster::Tanki {

template class PropCPUResourceManager <PropCPUResourceManagerRaylibBackend>;

}  // namespace MapMaster::Tanki

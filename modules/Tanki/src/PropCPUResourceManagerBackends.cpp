# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerR3DBackend.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerRaylibBackend.hpp"

# include "PropCPUResourceManager.inl" // IWYU pragma: keep

namespace MapMaster::Tanki {

template class PropCPUResourceManager <PropCPUResourceManagerR3DBackend>;
template class PropCPUResourceManager <PropCPUResourceManagerRaylibBackend>;

}  // namespace MapMaster::Tanki

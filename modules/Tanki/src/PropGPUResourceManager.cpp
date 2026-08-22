# include "MapMaster/Tanki/PropGPUResourceManager.hpp"
# include "MapMaster/Tanki/PropGPUResourceManagerR3DBackend.hpp"
# include "MapMaster/Tanki/PropGPUResourceManagerRaylibBackend.hpp"

# include "MapMaster/Tanki/PropGPUResourceManager.inl" // IWYU pragma: keep

namespace MapMaster::Tanki {

template class PropGPUResourceManager <PropGPUResourceManagerR3DBackend>;
template class PropGPUResourceManager <PropGPUResourceManagerRaylibBackend>;

}  // namespace MapMaster::Tanki

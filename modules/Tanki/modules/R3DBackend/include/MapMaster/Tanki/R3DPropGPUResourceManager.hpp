# pragma once

# include "MapMaster/Tanki/PropGPUResourceManager.hpp"
# include "MapMaster/Tanki/PropGPUResourceManagerR3DBackend.hpp"

namespace MapMaster::Tanki {

using R3DPropGPUResourceManager = PropGPUResourceManager <PropGPUResourceManagerR3DBackend>;

}  // namespace MapMaster::Tanki

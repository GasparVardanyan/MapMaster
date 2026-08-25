# pragma once

# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerR3DBackend.hpp"

namespace MapMaster::Tanki {

using R3DPropCPUResourceManager = PropCPUResourceManager <PropCPUResourceManagerR3DBackend>;

}  // namespace MapMaster::Tanki

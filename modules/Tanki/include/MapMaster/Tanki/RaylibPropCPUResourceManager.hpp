# pragma once

# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerRaylibBackend.hpp"

namespace MapMaster::Tanki {

using RaylibPropCPUResourceManager = PropCPUResourceManager <PropCPUResourceManagerRaylibBackend>;

}  // namespace MapMaster::Tanki

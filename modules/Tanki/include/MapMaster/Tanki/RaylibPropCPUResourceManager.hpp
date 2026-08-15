# pragma once

# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerRaylibAdapter.hpp"

namespace MapMaster::Tanki {

using RaylibPropCPUResourceManager = PropCPUResourceManager <PropCPUResourceManagerRaylibAdapter>;

}  // namespace MapMaster::Tanki

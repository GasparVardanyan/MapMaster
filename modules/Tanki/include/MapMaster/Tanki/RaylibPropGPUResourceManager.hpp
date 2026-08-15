# pragma once

# include "MapMaster/Tanki/PropGPUResourceManager.hpp"
# include "MapMaster/Tanki/PropGPUResourceManagerRaylibAdapter.hpp"

namespace MapMaster::Tanki {

using RaylibPropGPUResourceManager = PropGPUResourceManager <PropGPUResourceManagerRaylibAdapter>;

}  // namespace MapMaster::Tanki

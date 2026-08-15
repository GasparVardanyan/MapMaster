# pragma once

# include "MapMaster/Tanki/PropGPUResourceManager.hpp"

namespace MapMaster::Tanki {

using RaylibPropGPUResourceManager = PropGPUResourceManager <PropGPUResourceManagerRaylibAdapter>;

}  // namespace MapMaster::Tanki

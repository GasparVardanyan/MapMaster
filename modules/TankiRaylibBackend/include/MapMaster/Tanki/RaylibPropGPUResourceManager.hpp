# pragma once

# include "MapMaster/Tanki/PropGPUResourceManager.hpp"
# include "MapMaster/Tanki/PropGPUResourceManagerRaylibBackend.hpp"

namespace MapMaster::Tanki {

using RaylibPropGPUResourceManager = PropGPUResourceManager <PropGPUResourceManagerRaylibBackend>;

}  // namespace MapMaster::Tanki

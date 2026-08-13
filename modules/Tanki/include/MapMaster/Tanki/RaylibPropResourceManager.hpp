# pragma once

# include "MapMaster/Tanki/PropResourceManagerFrontend.hpp"

namespace MapMaster::Tanki {

using RaylibPropResourceManager = PropResourceManagerFrontend <PropResourceManagerRaylibAdapter>;

}  // namespace MapMaster::Tanki

# pragma once

# include "MapMaster/Tanki/MapRenderer.hpp"
# include "MapMaster/Tanki/MapRendererRaylibBackend.hpp"

namespace MapMaster::Tanki {

using RaylibMapRenderer = MapRenderer <MapRendererRaylibBackend>;

}  // namespace MapMaster::Tanki

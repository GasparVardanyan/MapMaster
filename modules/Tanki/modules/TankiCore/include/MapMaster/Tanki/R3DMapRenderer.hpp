# pragma once

# include "MapMaster/Tanki/MapRenderer.hpp"
# include "MapMaster/Tanki/MapRendererR3DBackend.hpp"

namespace MapMaster::Tanki {

using R3DMapRenderer = MapRenderer <MapRendererR3DBackend>;

}  // namespace MapMaster::Tanki

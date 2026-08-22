# include <chrono>
# include <iostream>
# include <memory>
# include <string>

# include <raylib.h>

# include "MapMaster/Utils/Tanki.hpp"
# include "MapMaster/Tanki/MapRendererR3DBackend.hpp"

namespace MapMaster::Tanki {
template <class MapRendererBackend>
class MapRenderer;
} // namespace MapMaster::Tanki

// using MapRendererBackend = MapMaster::Tanki::MapRendererRaylibBackend;
using MapRendererBackend = MapMaster::Tanki::MapRendererR3DBackend;

static constexpr float scale = 0.005F;

int main (int argc, char ** argv) {
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	MapMaster::Utils::Tanki::Window::OpenMapWindow <MapRendererBackend> (1680, 1050, "MapMaster", LOG_NONE);

	std::string propLibsRoot, mapFile;

	if (argc >= 3) {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		propLibsRoot = argv [1];
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		mapFile = argv [2];
	}
	else {
		return 0;
	}

	std::cout << "LOADING: " << propLibsRoot << " : " << mapFile << '\n';

	std::chrono::time_point start = std::chrono::steady_clock::now ();

	std::shared_ptr <MapMaster::Tanki::MapRenderer <MapRendererBackend>> rmap = MapMaster::Utils::Tanki::LoadMapRenderer <MapRendererBackend> (propLibsRoot, mapFile, scale, false);

	std::chrono::time_point end = std::chrono::steady_clock::now ();

	std::cout << "Elapsed: "
		<< std::chrono::duration <double> (end - start).count ()
		<< " s\n";

	MapMaster::Utils::Tanki::Window::DrawMapRendererInCurrentWindow <MapRendererBackend> (rmap, scale, propLibsRoot, mapFile);

	rmap.reset ();

	MapMaster::Utils::Tanki::Window::CloseMapWindow <MapRendererBackend> ();
}

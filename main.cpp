# include <chrono>
# include <iostream>
# include <memory>
# include <string>

# include <raylib.h>

# include "MapMaster/Utils/Tanki.hpp"

namespace MapMaster::Tanki {
class RaylibMap;
} // namespace MapMaster::Tanki



static constexpr float scale = 0.01F;

int main (int argc, char ** argv) {
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	MapMaster::Utils::Tanki::Window::OpenRaylibWindow (1680, 1050, "MapMaster", LOG_NONE);

	std::string propLibsRoot, mapFile;

	if (argc >= 3) {
		propLibsRoot = argv [1];// NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		mapFile = argv [2];
	}
	else {
		return 0;
	}

	std::cout << "LOADING: " << propLibsRoot << " : " << mapFile << '\n';

	std::chrono::time_point start = std::chrono::steady_clock::now ();

	std::shared_ptr <MapMaster::Tanki::RaylibMap> rmap = MapMaster::Utils::Tanki::LoadRaylibMap (propLibsRoot, mapFile, scale, false);

	std::chrono::time_point end = std::chrono::steady_clock::now ();

	std::cout << "Elapsed: "
		<< std::chrono::duration <double> (end - start).count ()
		<< " s\n";

	MapMaster::Utils::Tanki::Window::DrawRaylibMapInCurrentWindow (rmap, scale, propLibsRoot, mapFile);

	rmap.reset ();

	MapMaster::Utils::Tanki::Window::CloseRaylibWindow ();
}

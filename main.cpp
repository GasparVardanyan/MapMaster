# include <chrono>
# include <iostream>
# include <memory>
# include <string>
# include <vector>

# include <raylib.h>

# include "MapMaster/Tanki/RaylibMap.hpp"
# include "MapMaster/Tanki/RaylibPropResourceManager.hpp"
# include "MapMaster/Utils/Tanki.hpp"
# include "MapMaster/Tanki/Map.hpp"
# include "MapMaster/Tanki/PropResourceManagerFrontend.hpp"



static constexpr float scale = 0.01F;

int main (int argc, char ** argv) {
	MapMaster::Utils::Tanki::Window::OpenRaylibWindow (1680, 1050, "MapMaster", LOG_NONE);

	std::string propLibsRoot, mapFile;

	if (argc >= 3) {
		propLibsRoot = argv [1];
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

# include "MapMaster/Utils/Tanki.hpp"

# include <algorithm>
# include <filesystem>
# include <iostream>
# include <iterator>
# include <map>
# include <memory>
# include <stdexcept>
# include <string>
# include <utility>
# include <vector>
# include <functional>

# include <pugixml.hpp>
# include <raylib.h>

# include "MapMaster/Scene3D/CameraController.hpp"
# include "MapMaster/Tanki/Map.hpp"
# include "MapMaster/Tanki/PropLibrary.hpp"
# include "MapMaster/Tanki/PropResourceManager.hpp"
# include "MapMaster/Tanki/RaylibMap.hpp"
# include "MapMaster/Tanki/RaylibPropResourceManager.hpp"

using namespace MapMaster::Tanki;

namespace MapMaster::Utils::Tanki {

namespace Window {
void OpenRaylibWindow (int width, int height, const std::string & title, int logLevel) {
	SetTraceLogLevel (logLevel);
	SetConfigFlags (FLAG_MSAA_4X_HINT);
	InitWindow (width, height, title.c_str ());
}

void DrawRaylibMapInCurrentWindow (std::shared_ptr <MapMaster::Tanki::RaylibMap> rmap, float scale, const std::string & msg1, const std::string & msg2) {
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
	Camera camera = {
		.position = {
			.x = 50.0F,
			.y = 50.0F,
			.z = 50.0F,
		},
		.target = {
			.x = 0.0F, .y = 12.0F, .z = 0.0F
		},
		.up = {
			.x = 0.0F,
			.y = 0.0F,
			.z = 1.0F,
		},
		.fovy = 45.0F,
		.projection = CAMERA_PERSPECTIVE,
	};

	SetTargetFPS (60);

	bool drawCollisionGeometry = false;

	MapMaster::Scene3D::CameraController cameraController;
	cameraController.setCamera (& camera);
	cameraController.setMoveSpeed (cameraController.moveSpeed () * scale);

	while (false == WindowShouldClose ()) {
		// UpdateCamera (& camera, CAMERA_THIRD_PERSON);
		cameraController.updateCamera ();

		if (true == IsKeyPressed (KEY_SPACE)) {
			drawCollisionGeometry = !drawCollisionGeometry;
		}

		BeginDrawing ();

		ClearBackground ({.r = 0x22, .g = 0x44, .b = 0x66, .a = 0xFF});

		BeginMode3D (camera);

		if (true == drawCollisionGeometry) {
			rmap->renderCollisionGeometry ();
		}
		else {
			rmap->render (camera);

			// rlPushMatrix ();
			// rlRotatef (90.0F, 1.0F, 0.0F, 0.0F);
			// DrawGrid (50, 500.0F * scale);
			// rlPopMatrix ();
		}

		EndMode3D ();

		DrawFPS (10, 10);
		DrawText (msg1.c_str (), 10, 40, 20, GREEN);
		DrawText (msg2.c_str (), 10, 70, 20, GREEN);

		EndDrawing ();
	}

	// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

void CloseRaylibWindow () {
	CloseWindow ();
}
}  // namespace Window

std::shared_ptr <MapMaster::Tanki::RaylibMap> LoadRaylibMap (const std::string & libraryRootPath, const std::string & mapFile, float scale, bool haveCanonicalLibraryStructure) {
	std::shared_ptr <MapMaster::Tanki::RaylibMap> rmap = std::make_shared <MapMaster::Tanki::RaylibMap> ();

	if (true == haveCanonicalLibraryStructure) {
		// enable collision geometry loading
		rmap->setResourceManager (std::make_shared <MapMaster::Tanki::RaylibPropResourceManager> (true));

		// this loads the map xml data
		rmap->map ()->loadFile (mapFile);

		// this loads the xml data of the necessary proplibs from the given directory
		// all proplibs must be directly in this directory and their directories must
		// be named like their names specified in library.xml files
		//
		// otherwise manually load the directories with the loadLibrary method
		// rmap->resourceManager ()->loadMapLibraries (* rmap->map (), DATA_DIR "PLVK");
		rmap->resourceManager ()->loadMapLibraries (* rmap->map (), libraryRootPath);

		// this loads all the necessary mesh and texture resources to the gpu and
		// generates sprite info for rendering
		rmap->resourceManager ()->loadMapResources (* rmap->map ());

		// this creates the map scene
		rmap->loadScene (scale);
	}
	else
	{
		rmap->setResourceManager (std::make_shared <MapMaster::Tanki::RaylibPropResourceManager> (true));
		rmap->map ()->loadFile (mapFile);

		PropLibraryNameToPathVectorMap libraryInfo = FindPropLibraries (libraryRootPath);

		std::vector <std::string> mapLibrarySources = FindMapLibraries (libraryInfo, * rmap->map ());

		for (const std::string & source : mapLibrarySources) {
			rmap->resourceManager ()->loadLibrary (source);
		}

		rmap->resourceManager ()->loadMapResources (* rmap->map ());

		rmap->loadScene (scale);
	}

	return rmap;
}

PropLibraryNameToPathVectorMap FindPropLibraries (const std::string & libraryRootPath) {
	PropLibraryNameToPathVectorMap libraries;
	for (const std::filesystem::directory_entry & diEnt : std::filesystem::recursive_directory_iterator (libraryRootPath)) {
		if (true == diEnt.is_regular_file ()) {
			const std::filesystem::path & diEntPath = diEnt.path ();
			std::string diEntName = diEntPath.filename ().string ();

			if ("library.xml" == diEntName) {
				pugi::xml_document libXml;
				pugi::xml_parse_result pr = libXml.load_file (diEntPath.c_str ());

				if (true || pugi::xml_parse_status::status_ok == pr) {
					std::string libraryName = libXml.child ("library").attribute ("name").value ();

					libraries [libraryName].push_back (diEntPath.parent_path ().string ());
				}
				else {
					std::cerr << "Utils: PropLibrary failed to parse " << diEntPath << ". " << pr.description () << ".\n";
				}
			}
		}
	}

	return libraries;
}

PropLibraryNameToLibraryVectorMap LoadPropLibraries (const PropLibraryNameToPathVectorMap & nameToPathVectorMap) {
	PropLibraryNameToLibraryVectorMap libraries;

	std::ranges::transform (
		nameToPathVectorMap,
		std::inserter (libraries, libraries.end ()),
		[] (const std::pair <std::string, std::vector <std::string>> & entry)
			-> std::pair <std::string, std::vector <std::shared_ptr <MapMaster::Tanki::PropLibrary>>>
		{
			std::vector <std::shared_ptr <MapMaster::Tanki::PropLibrary>> libraries;

			std::ranges::transform (
				entry.second,
				std::back_inserter (libraries),
				[] (const std::string & path) -> std::shared_ptr <MapMaster::Tanki::PropLibrary> {
					std::shared_ptr <MapMaster::Tanki::PropLibrary> propLibrary = std::make_shared <MapMaster::Tanki::PropLibrary> ();
					propLibrary->loadDirectory (path);
					return propLibrary;
				}
			);

			return {entry.first, std::move (libraries)};
		}
	);

	return libraries;
}

std::shared_ptr <MapMaster::Tanki::PropResourceManager> LoadPropLibraryResources (const MapMaster::Tanki::PropLibrary & library) {
	std::shared_ptr <MapMaster::Tanki::PropResourceManager> resourceManager = std::make_shared <MapMaster::Tanki::PropResourceManager> ();

	resourceManager->addPropLibrary (std::make_shared <PropLibrary> (library));

	throw std::runtime_error ("this function must load all library resources");

	return resourceManager;
}

std::vector <std::string> FindMapLibraries (const PropLibraryNameToPathVectorMap & propLibraries, const Map & map) {
	std::vector <std::string> mapLibraries;

	for (const auto & [libraryName, groupData] : map.mapObjects ()) {
		if (true == propLibraries.contains (libraryName)) {
			const std::vector <std::string> & librarySources = propLibraries.at (libraryName);

			if (1 == librarySources.size ()) {
				mapLibraries.push_back (librarySources.back ());
			}
			if (1 < librarySources.size ()) {
				std::cerr << "WARN: " << librarySources.size () << " sources found for " << libraryName << '\n';
				for (const auto & x : librarySources) {
					std::cout << "\tsource: " << x << '\n';
				}
				// std::terminate ();
			}
		}
		else {
			std::cerr << "FAIL: no sources found for " << libraryName << '\n';
		}
	}

	return mapLibraries;
}

}  // namespace MapMaster::Utils::Tanki

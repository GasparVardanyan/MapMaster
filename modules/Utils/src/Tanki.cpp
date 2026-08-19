# include "MapMaster/Utils/Tanki.hpp"

# include <algorithm>
# include <exception>
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
# include <type_traits>

# include <pugixml.hpp>

# include <raylib.h>
# include <raymath.h>

# include <r3d/r3d_core.h>
# include <r3d/r3d_draw.h>
# include <r3d/r3d_lighting.h>
# include <r3d/r3d_material.h>
# include <r3d/r3d_environment.h>

# include "MapMaster/Scene3D/CameraController.hpp"
# include "MapMaster/Tanki/Map.hpp"
# include "MapMaster/Tanki/MapRendererR3DBackend.hpp"
# include "MapMaster/Tanki/MapRendererRaylibBackend.hpp"
# include "MapMaster/Tanki/PropLibrary.hpp"
# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/MapRenderer.hpp"
# include "MapMaster/Tanki/RaylibPropGPUResourceManager.hpp"
# include "MapMaster/Tanki/PropGPUResourceManager.hpp"

namespace MapMaster::Utils::Tanki {

namespace Window {
template <class MapRendererBackend>
void OpenMapWindow (int width, int height, const std::string & title, int logLevel) {
	if constexpr (
		   std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererRaylibBackend>
		|| std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererR3DBackend>
	) {
		SetTraceLogLevel (logLevel);
		SetConfigFlags (FLAG_MSAA_4X_HINT);
		InitWindow (width, height, title.c_str ());
		if constexpr (std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererR3DBackend>) {
			R3D_Init (width, height);
		}
	}
}

template <class MapRendererBackend>
void DrawMapRendererInCurrentWindow (std::shared_ptr <MapMaster::Tanki::MapRenderer <MapRendererBackend>> rmap, float scale, const std::string & msg1, const std::string & msg2) {
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


	R3D_ShadowMap map_r3d_shadow = {0};
	if constexpr (std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererR3DBackend>) {
		// Put this BEFORE you call R3D_LoadShadowMap()
		R3D_SetHint(R3D_HINT_SHADOW_DIR_SIZE, 4096); // 4096 is huge enough for a 42-unit map
		// 1. Create the shadow map. Because you use a directional light, use R3D_LIGHT_DIR.
		map_r3d_shadow = R3D_LoadShadowMap(R3D_LIGHT_DIR);

		// 1. Force all layers so distance doesn't cull the shadow
		map_r3d_shadow.cullMask = 0xFF; // 0xFF = All layers (R3D_LAYER_ALL)

		// 2. Ensure shadows are 100% solid, not faded
		map_r3d_shadow.opacity = 1.0f;

		// 2. Check if it loaded correctly
		if (!R3D_IsShadowMapValid(map_r3d_shadow)) {
			std::exit (-1);
		}
	}

	if constexpr (std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererR3DBackend>) {
		// 1. Ambient: Rock bottom energy, pure sky-blue color.
		R3D_GetEnvironment()->ambient.color = (Color){ 100, 140, 200, 255 }; // Clear sky blue
		R3D_GetEnvironment()->ambient.energy = 0.08f; // 8% brightness. Shadows are dark, not gray.
	}

	R3D_Light* map_r3d_light = nullptr;
	if constexpr (std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererR3DBackend>) {
		map_r3d_light = new R3D_Light;
		// 2. Directional Sun: Pure warm afternoon sun, high energy.
		Vector3 sunDir = { -1.0f, -1.0f, -1.0f };
		Color sunColor = { 255, 230, 185, 255 }; // Warm Yellow/Orange
		*map_r3d_light = R3D_CreateDirLight(sunDir, sunColor, 0.95f); // Almost 100%
	}

	while (false == WindowShouldClose ()) {
		// UpdateCamera (& camera, CAMERA_THIRD_PERSON);
		cameraController.updateCamera ();

		if (true == IsKeyPressed (KEY_SPACE)) {
			drawCollisionGeometry = !drawCollisionGeometry;
		}

		BeginDrawing ();
		if constexpr (std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererR3DBackend>) {
			R3D_Begin(camera);
			// R3D_PushLight(* map_r3d_light);
			R3D_PushLightEx(*map_r3d_light, map_r3d_shadow, true);
		}
		else {
			ClearBackground ({.r = 0x22, .g = 0x44, .b = 0x66, .a = 0xFF});
			BeginMode3D (camera);
		}

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

		if constexpr (std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererR3DBackend>) {
			R3D_End();
		}
		else {
			EndMode3D ();
		}

		DrawFPS (10, 10);
		DrawText (msg1.c_str (), 10, 40, 20, GREEN);
		DrawText (msg2.c_str (), 10, 70, 20, GREEN);

		EndDrawing ();
	}

	delete map_r3d_light;

	// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

template <class MapRendererBackend>
void CloseMapWindow () {
	if constexpr (
		   std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererRaylibBackend>
		|| std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererR3DBackend>
	) {
		if constexpr (std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererR3DBackend>) {
			R3D_Close();
		}

		CloseWindow ();
	}
}

}  // namespace Window

template <class MapRendererBackend>
std::shared_ptr <MapMaster::Tanki::MapRenderer <MapRendererBackend>> LoadMapRenderer (const std::string & libraryRootPath, const std::string & mapFile, float scale, bool haveCanonicalLibraryStructure) {
	std::shared_ptr <MapMaster::Tanki::MapRenderer <MapRendererBackend>> rmap = std::make_shared <MapMaster::Tanki::MapRenderer <MapRendererBackend>> ();

	if (true || true == haveCanonicalLibraryStructure) {
		// enable collision geometry loading
		rmap->setResourceManager (std::make_shared <typename MapRendererBackend::GPUResourceManager> (true));

		// this loads the map xml data
		rmap->map ()->loadFile (mapFile);

		// this loads the xml data of the necessary proplibs from the given directory
		// all proplibs must be directly in this directory and their directories must
		// be named like their names specified in library.xml files
		//
		// otherwise manually load the directories with the loadLibrary method
		rmap->resourceManager ()->loadMapLibraries (* rmap->map (), libraryRootPath);

		// this loads all the necessary mesh and texture resources to the gpu and
		// generates sprite info for rendering
		rmap->resourceManager ()->loadMapResources (* rmap->map ());

		// this creates the map scene
		rmap->loadScene (scale);
	}
	else
	{
		// rmap->setResourceManager (std::make_shared <MapMaster::Tanki::RaylibPropGPUResourceManager> (true));
		// rmap->map ()->loadFile (mapFile);
		//
		// PropLibraryNameToPathVectorMap libraryInfo = FindPropLibraryPaths (libraryRootPath);
		//
		// std::vector <std::string> mapLibrarySources = FindMapLibraryPaths (libraryInfo, * rmap->map ());
		//
		// for (const std::string & source : mapLibrarySources) {
		// 	rmap->resourceManager ()->loadLibrary (source);
		// }
		//
		// rmap->resourceManager ()->loadMapResources (* rmap->map ());
		//
		// rmap->loadScene (scale);
	}

	return rmap;
}

// PropLibraryNameToPathVectorMap FindPropLibraryPaths (const std::string & libraryRootPath) {
// 	PropLibraryNameToPathVectorMap libraries;
// 	for (const std::filesystem::directory_entry & diEnt : std::filesystem::recursive_directory_iterator (libraryRootPath)) {
// 		if (true == diEnt.is_regular_file ()) {
// 			const std::filesystem::path & diEntPath = diEnt.path ();
// 			std::string diEntName = diEntPath.filename ().string ();
//
// 			if ("library.xml" == diEntName) {
// 				pugi::xml_document libXml;
// 				pugi::xml_parse_result pr = libXml.load_file (diEntPath.c_str ());
//
// 				if (pugi::xml_parse_status::status_ok == pr.status) {
// 					std::string libraryName = libXml.child ("library").attribute ("name").value ();
//
// 					libraries [libraryName].push_back (diEntPath.parent_path ().string ());
// 				}
// 				else {
// 					std::cerr << "Utils: PropLibrary failed to parse " << diEntPath << ". " << pr.description () << ".\n";
// 				}
// 			}
// 		}
// 	}
//
// 	return libraries;
// }
//
// PropLibraryNameToLibraryVectorMap LoadPropLibraries (const PropLibraryNameToPathVectorMap & nameToPathVectorMap) {
// 	PropLibraryNameToLibraryVectorMap libraries;
//
// 	std::ranges::transform (
// 		nameToPathVectorMap,
// 		std::inserter (libraries, libraries.end ()),
// 		[] (const std::pair <std::string, std::vector <std::string>> & entry)
// 			-> std::pair <std::string, std::vector <std::shared_ptr <MapMaster::Tanki::PropLibrary>>>
// 		{
// 			std::vector <std::shared_ptr <MapMaster::Tanki::PropLibrary>> libraries;
//
// 			std::ranges::transform (
// 				entry.second,
// 				std::back_inserter (libraries),
// 				[] (const std::string & path) -> std::shared_ptr <MapMaster::Tanki::PropLibrary> {
// 					std::shared_ptr <MapMaster::Tanki::PropLibrary> propLibrary = std::make_shared <MapMaster::Tanki::PropLibrary> ();
// 					propLibrary->loadDirectory (path);
// 					return propLibrary;
// 				}
// 			);
//
// 			return {entry.first, std::move (libraries)};
// 		}
// 	);
//
// 	return libraries;
// }
//
// PropLibraryNameToResourceManagerVectorMap LoadPropLibraryResources(const PropLibraryNameToLibraryVectorMap & nameToLibraryVectorMap) {
// 	PropLibraryNameToResourceManagerVectorMap libraries;
//
// 	std::ranges::transform (
// 		nameToLibraryVectorMap,
// 		std::inserter (libraries, libraries.end ()),
// 		[] (const std::pair <std::string, std::vector <std::shared_ptr <MapMaster::Tanki::PropLibrary>>> & entry)
// 			-> std::pair <std::string, std::vector <std::shared_ptr <MapMaster::Tanki::PropCPUResourceManager>>>
// 		{
// 			std::vector <std::shared_ptr <MapMaster::Tanki::PropCPUResourceManager>> resources;
//
// 			std::ranges::transform (
// 				entry.second,
// 				std::back_inserter (resources),
// 				static_cast <
// 					std::shared_ptr <MapMaster::Tanki::PropCPUResourceManager> (*) (std::shared_ptr <MapMaster::Tanki::PropLibrary>)
// 				> (& LoadPropLibraryResources)
// 			);
//
// 			return {entry.first, std::move (resources)};
// 		}
// 	);
//
// 	return libraries;
// }
//
// std::shared_ptr <MapMaster::Tanki::PropCPUResourceManager> LoadPropLibraryResources (std::shared_ptr <MapMaster::Tanki::PropLibrary> library) {
// 	std::shared_ptr <MapMaster::Tanki::PropCPUResourceManager> resourceManager = std::make_shared <MapMaster::Tanki::PropCPUResourceManager> ();
//
// 	resourceManager->addPropLibrary (std::move (library));
//
// 	throw std::runtime_error ("this function must load all library resources");
//
// 	return resourceManager;
// }
//
// std::vector <std::string> FindMapLibraryPaths (const PropLibraryNameToPathVectorMap & propLibraries, const MapMaster::Tanki::Map & map) {
// 	std::vector <std::string> mapLibraryPaths;
//
// 	const MapMaster::Tanki::Map::MapObjectCollection & mapObjects = map.mapObjects ();
// 	mapLibraryPaths.reserve (mapObjects.size ());
//
// 	for (const auto & [libraryName, groupData] : mapObjects) {
// 		if (true == propLibraries.contains (libraryName)) {
// 			const std::vector <std::string> & librarySources = propLibraries.at (libraryName);
//
// 			if (1 == librarySources.size ()) {
// 				mapLibraryPaths.push_back (librarySources.back ());
// 			}
// 			else {
// 				std::cerr << "ERROR: " << librarySources.size () << " sources found for " << libraryName << '\n';
// 				for (const auto & x : librarySources) {
// 					std::cout << "\tsource: " << x << '\n';
// 				}
// 				std::terminate ();
// 			}
// 		}
// 		else {
// 			std::cerr << "FAIL: no sources found for " << libraryName << '\n';
// 		}
// 	}
//
// 	return mapLibraryPaths;
// }
//
// std::vector <std::string> FindMapLibraryNames (const MapMaster::Tanki::Map & map) {
// 	std::vector <std::string> mapLibraryNames;
//
// 	const MapMaster::Tanki::Map::MapObjectCollection & mapObjects = map.mapObjects ();
// 	mapLibraryNames.resize (mapObjects.size ());
//
// 	std::ranges::transform (mapObjects, mapLibraryNames.begin (), [] (const auto & mapObject) -> std::string {
// 		return mapObject.first;
// 	});
//
// 	return mapLibraryNames;
// }

namespace Window {

template void OpenMapWindow <MapMaster::Tanki::MapRendererRaylibBackend> (int width, int height, const std::string & title, int logLevel);
template void OpenMapWindow <MapMaster::Tanki::MapRendererR3DBackend> (int width, int height, const std::string & title, int logLevel);


template void CloseMapWindow <MapMaster::Tanki::MapRendererRaylibBackend> ();
template void CloseMapWindow <MapMaster::Tanki::MapRendererR3DBackend> ();

template void DrawMapRendererInCurrentWindow (std::shared_ptr <MapMaster::Tanki::MapRenderer <MapMaster::Tanki::MapRendererRaylibBackend>> rmap, float scale, const std::string & msg1, const std::string & msg2);
template void DrawMapRendererInCurrentWindow (std::shared_ptr <MapMaster::Tanki::MapRenderer <MapMaster::Tanki::MapRendererR3DBackend>> rmap, float scale, const std::string & msg1, const std::string & msg2);

} // end namespace Window

template std::shared_ptr <MapMaster::Tanki::MapRenderer <MapMaster::Tanki::MapRendererRaylibBackend>> LoadMapRenderer (const std::string & libraryRootPath, const std::string & mapFile, float scale, bool haveCanonicalLibraryStructure);
template std::shared_ptr <MapMaster::Tanki::MapRenderer <MapMaster::Tanki::MapRendererR3DBackend>> LoadMapRenderer (const std::string & libraryRootPath, const std::string & mapFile, float scale, bool haveCanonicalLibraryStructure);


}  // namespace MapMaster::Utils::Tanki

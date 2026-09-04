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
# include <r3d/r3d_core.h>
# include <r3d/r3d_draw.h>
# include <r3d/r3d_lighting.h>
# include <r3d/r3d_environment.h>

# include "MapMaster/Scene3D/CameraController.hpp"
# include "MapMaster/Tanki/Map.hpp"
# include "MapMaster/Tanki/MapRenderer.hpp" // IWYU pragma: keep
# include "MapMaster/Tanki/MapRendererR3DBackend.hpp" // IWYU pragma: keep
# include "MapMaster/Tanki/MapRendererRaylibBackend.hpp" // IWYU pragma: keep
# include "MapMaster/Tanki/PropLibrary.hpp"

namespace MapMaster::Utils::Tanki {

namespace Window {
template <class MapRendererBackend>
void OpenMapWindow (const std::string & title, int logLevel, float scale) {
	if constexpr (
		   std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererRaylibBackend>
		|| std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererR3DBackend>
	) {
		SetTraceLogLevel (logLevel);
		SetConfigFlags (FLAG_FULLSCREEN_MODE);
		SetConfigFlags (FLAG_MSAA_4X_HINT);
		InitWindow (0, 0, title.c_str ());
		if constexpr (std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererR3DBackend>) {
			R3D_Init (GetScreenWidth (), GetScreenHeight ());

			R3D_ENVIRONMENT_SET (ssao.enabled, true);
			R3D_ENVIRONMENT_SET (ssao.radius, 1.0F * scale);
			R3D_ENVIRONMENT_SET (bloom.mode, R3D_Bloom::R3D_BLOOM_SCREEN);
			R3D_ENVIRONMENT_SET (bloom.mode, R3D_Bloom::R3D_BLOOM_SCREEN);
			// R3D_ENVIRONMENT_SET (ssr.enabled, true);
			R3D_ENVIRONMENT_SET (fog.mode, R3D_Fog::R3D_FOG_EXP2);
			R3D_ENVIRONMENT_SET (fog.density, 0.0065);
			R3D_ENVIRONMENT_SET (fog.color, (Color) { .r = 0x4A, .g = 0x3A, .b = 0x5A, .a = 0xFF });

			R3D_ENVIRONMENT_SET (ambient.color, (Color) { .r = 0x80, .g = 0x70, .b = 0x90, .a = 0xFF });
			R3D_ENVIRONMENT_SET (ambient.energy, 0.35);
			R3D_ENVIRONMENT_SET (background.color, (Color) {.r = 0x22, .g = 0x44, .b = 0x66, .a = 0xFF});

			R3D_SetAntiAliasingMode (R3D_AntiAliasingMode::R3D_ANTI_ALIASING_MODE_SMAA);
			R3D_SetAntiAliasingPreset (R3D_AntiAliasingPreset::R3D_ANTI_ALIASING_PRESET_ULTRA);
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
		map_r3d_shadow = R3D_LoadShadowMap (R3D_LIGHT_OMNI);
		map_r3d_shadow.cullMask = 0xFF;
		map_r3d_shadow.opacity = 1.0f;
	}

	R3D_Light* map_r3d_light = nullptr;
	if constexpr (std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererR3DBackend>) {
		map_r3d_light = new R3D_Light;
		// * map_r3d_light = R3D_CreateDirLight (
		// 	{ -1.0f, -1.0f, -1.0f },
		// 	WHITE,
		// 	0.8
		// );
		// *map_r3d_light = R3D_CreateDirLight (
		// 		{ -0.35f, -0.75f, -0.25f },
		// 		(Color) {
		// 		.r = 0xFF,
		// 		.g = 0xB0,
		// 		.b = 0x70,
		// 		.a = 0xFF
		// 		},
		// 		0.7f
		// 		);
		* map_r3d_light = R3D_CreateOmniLight (
			{ 3000 * scale, 9000 * scale, 9000 * scale },
			10000,
			(Color) { .r = 0xFF, .g = 0xB0, .b = 0x70, .a = 0xFF }, 0.7F
		);
	}

	while (false == WindowShouldClose ()) {
		// UpdateCamera (& camera, CAMERA_THIRD_PERSON);
		cameraController.updateCamera ();

		if (true == IsKeyPressed (KEY_SPACE)) {
			drawCollisionGeometry = !drawCollisionGeometry;
		}

		BeginDrawing ();

		if (false == drawCollisionGeometry) {
			if constexpr (std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererR3DBackend>) {
				R3D_Begin (camera);
				R3D_PushLightEx (* map_r3d_light, map_r3d_shadow, true);
				// R3D_PushLight (* map_r3d_light);
			}
			else {
				ClearBackground ({.r = 0x22, .g = 0x44, .b = 0x66, .a = 0xFF});
				BeginMode3D (camera);
			}

			rmap->render (camera);

			if constexpr (std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererR3DBackend>) {
				R3D_End ();
			}
			else {
				EndMode3D ();
			}

			// rlPushMatrix ();
			// rlRotatef (90.0F, 1.0F, 0.0F, 0.0F);
			// DrawGrid (50, 500.0F * scale);
			// rlPopMatrix ();
		}
		else {
			ClearBackground ({.r = 0x22, .g = 0x44, .b = 0x66, .a = 0xFF});
			BeginMode3D (camera);

			rmap->renderCollisionGeometry ();

			EndMode3D ();
		}

		DrawFPS (10, 10);
		DrawText (msg1.c_str (), 10, 40, 20, GREEN);
		DrawText (msg2.c_str (), 10, 70, 20, GREEN);

		EndDrawing ();
	}

	if constexpr (std::is_same_v <MapRendererBackend, MapMaster::Tanki::MapRendererR3DBackend>) {
		delete map_r3d_light;
	}

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

	if (true == haveCanonicalLibraryStructure) {
		// enable collision geometry loading
		rmap->setResourceManager (std::make_shared <typename MapRendererBackend::GPUResourceManager> (true, true));

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
		rmap->setResourceManager (std::make_shared <typename MapRendererBackend::GPUResourceManager> (true));
		rmap->map ()->loadFile (mapFile);

		PropLibraryNameToPathVectorMap libraryInfo = FindPropLibraryPaths (libraryRootPath);

		std::vector <std::string> mapLibrarySources = FindMapLibraryPaths (libraryInfo, * rmap->map ());

		for (const std::string & source : mapLibrarySources) {
			rmap->resourceManager ()->loadLibrary (source);
		}

		rmap->resourceManager ()->loadMapResources (* rmap->map ());

		rmap->loadScene (scale);
	}

	return rmap;
}

PropLibraryNameToPathVectorMap FindPropLibraryPaths (const std::string & libraryRootPath) {
	PropLibraryNameToPathVectorMap libraries;
	for (const std::filesystem::directory_entry & diEnt : std::filesystem::recursive_directory_iterator (libraryRootPath)) {
		if (true == diEnt.is_regular_file ()) {
			const std::filesystem::path & diEntPath = diEnt.path ();
			std::string diEntName = diEntPath.filename ().string ();

			if ("library.xml" == diEntName) {
				pugi::xml_document libXml;
				pugi::xml_parse_result pr = libXml.load_file (diEntPath.c_str ());

				if (pugi::xml_parse_status::status_ok == pr.status) {
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

template <class PropCPUResourceManagerBackend>
PropLibraryNameToResourceManagerVectorMap <PropCPUResourceManagerBackend> LoadPropLibraryResources (const PropLibraryNameToLibraryVectorMap & nameToLibraryVectorMap) {
	PropLibraryNameToResourceManagerVectorMap <PropCPUResourceManagerBackend> libraries;

	std::ranges::transform (
		nameToLibraryVectorMap,
		std::inserter (libraries, libraries.end ()),
		[] (const std::pair <std::string, std::vector <std::shared_ptr <MapMaster::Tanki::PropLibrary>>> & entry)
			-> std::pair <std::string, std::vector <std::shared_ptr <MapMaster::Tanki::PropCPUResourceManager <PropCPUResourceManagerBackend>>>>
		{
			std::vector <std::shared_ptr <MapMaster::Tanki::PropCPUResourceManager <PropCPUResourceManagerBackend>>> resources;

			std::ranges::transform (
				entry.second,
				std::back_inserter (resources),
				static_cast <
					std::shared_ptr <MapMaster::Tanki::PropCPUResourceManager <PropCPUResourceManagerBackend>> (*) (std::shared_ptr <MapMaster::Tanki::PropLibrary>)
				> (& LoadPropLibraryResources)
			);

			return {entry.first, std::move (resources)};
		}
	);

	return libraries;
}

template <class PropCPUResourceManagerBackend>
std::shared_ptr <MapMaster::Tanki::PropCPUResourceManager <PropCPUResourceManagerBackend>> LoadPropLibraryResources (std::shared_ptr <MapMaster::Tanki::PropLibrary> library) {
	std::shared_ptr <MapMaster::Tanki::PropCPUResourceManager <PropCPUResourceManagerBackend>> resourceManager = std::make_shared <MapMaster::Tanki::PropCPUResourceManager <PropCPUResourceManagerBackend>> ();

	resourceManager->addPropLibrary (std::move (library));

	throw std::runtime_error ("this function must load all library resources");

	return resourceManager;
}

std::vector <std::string> FindMapLibraryPaths (const PropLibraryNameToPathVectorMap & propLibraries, const MapMaster::Tanki::Map & map) {
	std::vector <std::string> mapLibraryPaths;

	const MapMaster::Tanki::Map::MapObjectCollection & mapObjects = map.mapObjects ();
	mapLibraryPaths.reserve (mapObjects.size ());

	for (const auto & [libraryName, groupData] : mapObjects) {
		if (true == propLibraries.contains (libraryName)) {
			const std::vector <std::string> & librarySources = propLibraries.at (libraryName);

			if (1 == librarySources.size ()) {
				mapLibraryPaths.push_back (librarySources.back ());
			}
			else {
				std::cerr << "ERROR: " << librarySources.size () << " sources found for " << libraryName << '\n';
				for (const auto & x : librarySources) {
					std::cout << "\tsource: " << x << '\n';
				}
				std::terminate ();
			}
		}
		else {
			std::cerr << "FAIL: no sources found for " << libraryName << '\n';
		}
	}

	return mapLibraryPaths;
}

std::vector <std::string> FindMapLibraryNames (const MapMaster::Tanki::Map & map) {
	std::vector <std::string> mapLibraryNames;

	const MapMaster::Tanki::Map::MapObjectCollection & mapObjects = map.mapObjects ();
	mapLibraryNames.resize (mapObjects.size ());

	std::ranges::transform (mapObjects, mapLibraryNames.begin (), [] (const auto & mapObject) -> std::string {
		return mapObject.first;
	});

	return mapLibraryNames;
}

namespace Window {

template void OpenMapWindow <MapMaster::Tanki::MapRendererRaylibBackend> (const std::string & title, int logLevel, float scale);
template void OpenMapWindow <MapMaster::Tanki::MapRendererR3DBackend> (const std::string & title, int logLevel, float scale);


template void CloseMapWindow <MapMaster::Tanki::MapRendererRaylibBackend> ();
template void CloseMapWindow <MapMaster::Tanki::MapRendererR3DBackend> ();

template void DrawMapRendererInCurrentWindow (std::shared_ptr <MapMaster::Tanki::MapRenderer <MapMaster::Tanki::MapRendererRaylibBackend>> rmap, float scale, const std::string & msg1, const std::string & msg2);
template void DrawMapRendererInCurrentWindow (std::shared_ptr <MapMaster::Tanki::MapRenderer <MapMaster::Tanki::MapRendererR3DBackend>> rmap, float scale, const std::string & msg1, const std::string & msg2);

} // end namespace Window

template std::shared_ptr <MapMaster::Tanki::MapRenderer <MapMaster::Tanki::MapRendererRaylibBackend>> LoadMapRenderer (const std::string & libraryRootPath, const std::string & mapFile, float scale, bool haveCanonicalLibraryStructure);
template std::shared_ptr <MapMaster::Tanki::MapRenderer <MapMaster::Tanki::MapRendererR3DBackend>> LoadMapRenderer (const std::string & libraryRootPath, const std::string & mapFile, float scale, bool haveCanonicalLibraryStructure);

template PropLibraryNameToResourceManagerVectorMap <MapMaster::Tanki::PropCPUResourceManagerRaylibBackend> LoadPropLibraryResources (const PropLibraryNameToLibraryVectorMap & nameToLibraryVectorMap);
template PropLibraryNameToResourceManagerVectorMap <MapMaster::Tanki::PropCPUResourceManagerR3DBackend> LoadPropLibraryResources (const PropLibraryNameToLibraryVectorMap & nameToLibraryVectorMap);
template std::shared_ptr <MapMaster::Tanki::PropCPUResourceManager <MapMaster::Tanki::PropCPUResourceManagerRaylibBackend>> LoadPropLibraryResources (std::shared_ptr <MapMaster::Tanki::PropLibrary> library);
template std::shared_ptr <MapMaster::Tanki::PropCPUResourceManager <MapMaster::Tanki::PropCPUResourceManagerR3DBackend>> LoadPropLibraryResources (std::shared_ptr <MapMaster::Tanki::PropLibrary> library);


}  // namespace MapMaster::Utils::Tanki

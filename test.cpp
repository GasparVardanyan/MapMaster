# include <algorithm>
# include <iostream>
# include <memory>
# include <numbers>

# define SUPPORT_FILEFORMAT_JPG 1

# include <chrono>
# include <cmath>
# include <map>
# include <string>
# include <utility>
# include <vector>

# include <raylib.h>

# include "Map.hpp"
# include "PropLibrary.hpp"
# include "PropRaylibResourceManager.hpp"
# include "PropResourceManager.hpp"



static constexpr float scale = 0.01F;

// NOLINTBEGIN(*)
int main (void)
{
	const int screenWidth = 800;
	const int screenHeight = 450;

	// SetTraceLogLevel (LOG_NONE);
	SetConfigFlags (FLAG_MSAA_4X_HINT);
	InitWindow (screenWidth, screenHeight, "MapMaster");
	auto start = std::chrono::steady_clock::now ();

	Map map;
	map.loadFile (DATA_DIR "maps/M/map_silence_remake_cy95v_summer/map.xml");
	// map.loadFile (DATA_DIR "finalboss.xml");
	// map.loadFile (DATA_DIR "maps/Summer/Sandbox_MM.xml");
	// map.loadFile (DATA_DIR "maps/M/map_sandbox_2.0_summer/map.xml");
	// map.loadFile (DATA_DIR "maps/M/map_tutorial_summer/map.xml");
	// map.loadFile (DATA_DIR "map.xml");

	PropRaylibResourceManager raylibResManager;
	const PropResourceManager & resourceManager = raylibResManager.resourceManager ();

	raylibResManager.loadMapLibraries (map, DATA_DIR "propslibs");
	raylibResManager.loadMapResources (map);

	const auto & meshResources = raylibResManager.meshResources ();
	const auto & textureResources = raylibResManager.textureResources ();
	const auto & spriteInfos = raylibResManager.spriteInfos ();

	const auto & libraries = resourceManager.propLibraries ();

	struct SceneMesh {
		Vector3 position = {};
		Vector3 rotation = {};
		std::string library;
		std::string meshFile;
		std::string textureFile;
	};

	std::vector <SceneMesh> sceneMeshes;

	struct SceneSprite {
		Vector3 position = {};
		std::string library;
		std::string textureFile; // FIXME: use propName
	};

	std::vector <SceneSprite> sceneSprites;

	for (const auto & [libraryName, groups] : map.mapObjects ()) {
		const auto & library = libraries.at (libraryName);
		for (const auto & [groupName, props] : groups) {
			const auto & group = library.groups ().at (groupName);
			for (const auto & [propName, propInfo] : props) {
				if (true == group.meshes.contains (propName)) {
					const PropResourceManager::PropMeshResource & meshResource = const_cast <PropResourceManager::PropMeshResource &> (resourceManager.getMeshResource (libraryName, groupName, propName));
					const std::string meshFile = resourceManager.propLibraries ().at (libraryName).groups ().at (groupName).meshes.at (propName).file;

					for (const auto & prop : propInfo) {
						std::string textureName = prop.textureName;
						std::string textureFile;
						if (true == textureName.empty ()) {
							textureFile = meshResource.textureFile;
						}
						else {
							textureFile = resourceManager.propLibraries ().at (libraryName).groups ().at (groupName).meshes.at (propName).textures.at (textureName);
						}

						textureFile = resourceManager.propLibraries ().at (libraryName).getActualTextureFileName (textureFile);

						sceneMeshes.push_back ({
							.library = libraryName,
							.meshFile = meshFile,
							.textureFile = textureFile,
							.position = {scale * static_cast<float>(prop.positionX), scale * static_cast<float>(prop.positionY), scale * static_cast<float>(prop.positionZ)},
							.rotation = {0, 0, static_cast<float>(prop.rotationZ * 180 / std::numbers::pi)}
						});
					}
				}
				else if (true == group.sprites.contains (propName)) {
					const auto & sprite = group.sprites.at (propName);
					std::string textureFile = resourceManager.propLibraries ().at (libraryName).getActualTextureFileName (sprite.diffuseFile);

					for (const auto & prop : propInfo) {
						sceneSprites.push_back ({
							.library = libraryName,
							.position = {scale * static_cast<float>(prop.positionX), scale * static_cast<float>(prop.positionY), scale * static_cast<float>(prop.positionZ)},
							.textureFile = textureFile
						});
					}
				}
			}
		}
	}


	auto end = std::chrono::steady_clock::now ();

	// raylibResManager = {};
	// resManager = {};

	std::cout << "Elapsed: "
		<< std::chrono::duration<double>(end - start).count ()
		<< " s\n";


	Camera camera = { 0 };
	camera.position = { 50.0f, 50.0f, 50.0f };
	camera.target = { 0.0f, 12.0f, 0.0f };
	camera.up = { 0.0f, 0.0f, 1.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	SetTargetFPS (60);

	std::vector <std::pair <std::string, std::string>> selectedMeshes;
	std::vector <std::pair <std::string, std::string>> selectedSprites;

	while (!WindowShouldClose ())
	{
		UpdateCamera (& camera, CAMERA_THIRD_PERSON);

		BeginDrawing ();

		ClearBackground (RAYWHITE);

		BeginMode3D (camera);

		for (const auto & mesh : sceneMeshes) {
			auto & model = meshResources.at (mesh.library).at (mesh.meshFile).model;
			auto & texture = textureResources.at (mesh.library).at (mesh.textureFile).texture;
			model.materials [0].maps [MATERIAL_MAP_DIFFUSE].texture = texture;
			Color tint = WHITE;
			if (selectedMeshes.cend () != std::find (selectedMeshes.cbegin (), selectedMeshes.cend (), std::pair <std::string, std::string> (mesh.library, mesh.meshFile))) {
				tint = RED;
			}
			DrawModelEx (model, mesh.position, {0, 0, 1}, mesh.rotation.z, {scale, scale, scale}, tint);
		}

		for (const auto & sprite : sceneSprites) {
			auto & texture = textureResources.at (sprite.library).at (sprite.textureFile).texture;
			auto & spdata = spriteInfos.at (sprite.library).at (sprite.textureFile);

			DrawBillboardPro (
				camera,
				textureResources.at (sprite.library).at (sprite.textureFile).texture,
				{0, 0, static_cast <float> (texture.width), static_cast <float> (texture.height)},
				sprite.position,
				{0, 0, 1},
				{ spdata.size.x * scale, spdata.size.y * scale },
				{ spdata.origin.x * scale, spdata.origin.y * scale },
				0,
				WHITE
			);
		}

		// DrawGrid (20, 10.0f);

		EndMode3D ();

		DrawFPS (10, 10);

		EndDrawing ();
	}

	// UnloadTexture (texture);

	CloseWindow ();

	return 0;
}
// NOLINTEND(*)


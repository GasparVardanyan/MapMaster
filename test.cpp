# define SUPPORT_FILEFORMAT_JPG 1

# include <algorithm>
# include <chrono>
# include <iostream>
# include <map>
# include <numbers>
# include <string>
# include <utility>
# include <vector>

# include <raylib.h>
# include <rlgl.h>

# include "Map.hpp"
# include "PropLibrary.hpp"
# include "PropResourceManager.hpp"
# include "RaylibPropResourceManager.hpp"



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

	struct SceneMeshDescriptor {
		std::string libraryName;
		std::string groupName;
		std::string propName;
		std::string textureName;
	};

	struct SceneSpriteDescriptor {
		std::string libraryName;
		std::string groupName;
		std::string propName;
		std::string textureName;
	};

	struct SceneMesh {
		SceneMeshDescriptor descriptor;
		Vector3 position = {};
		Vector3 rotation = {};
		Vector3 scale = {};
		const Model & model;
		const Texture2D & texture;
		Color tint = WHITE;
	};

	struct SceneSprite {
		SceneSpriteDescriptor descriptor;
		Vector3 position = {};
		Rectangle rect = {};
		Vector2 size = {};
		Vector2 origin = {};
		const Texture2D & texture;
		Color tint = WHITE;
	};

	std::vector <SceneMesh> sceneMeshes;
	std::vector <SceneSprite> sceneSprites;




	Map map;
	// map.loadFile (DATA_DIR "maps/M/map_silence_remake_cy95v_summer/map.xml");
	// map.loadFile (DATA_DIR "finalboss.xml");
	// map.loadFile (DATA_DIR "maps/Summer/Sandbox_MM.xml");
	map.loadFile (DATA_DIR "maps/M/map_sandbox_2.0_summer/map.xml");
	// map.loadFile (DATA_DIR "maps/M/map_tutorial_summer/map.xml");
	// map.loadFile (DATA_DIR "map.xml");

	RaylibPropResourceManager raylibResManager;

	raylibResManager.loadMapLibraries (map, DATA_DIR "propslibs");
	raylibResManager.loadMapResources_OLD (map);

	const auto & raylibMeshResources = raylibResManager.meshResources ();
	const auto & raylibTextureResources = raylibResManager.textureResources ();
	const auto & raylibSpriteInfos = raylibResManager.spriteInfos ();

	const PropResourceManager & resourceManager = raylibResManager.resourceManager ();
	const auto & propLibraries = resourceManager.propLibraries ();
	const auto & meshResources = resourceManager.propMeshResources ();





	for (const auto & [libraryName, groups] : map.mapObjects ()) {
		const PropLibrary & library = * propLibraries.at (libraryName);
		const std::map <std::string, PropLibrary::Group> & libraryGroups = library.groups ();



		for (const auto & [groupName, props] : groups) {
			const PropLibrary::Group & group = libraryGroups.at (groupName);



			for (const auto & [propName, propInfo] : props) {
				if (true == group.meshes.contains (propName)) {

					const PropResourceManager::PropMeshResource & meshResource = const_cast <PropResourceManager::PropMeshResource &> (resourceManager.getMeshResource (libraryName, groupName, propName));
					const PropLibrary::PropMesh & propMesh = group.meshes.at (propName);
					const std::string meshFile = propMesh.file;

					for (const Map::MapObject & mapObject : propInfo) {
						std::string textureName = mapObject.textureName;
						std::string textureFile;
						if (true == textureName.empty ()) {
							textureFile = meshResource.textureFile;
						}
						else {
							textureFile = propMesh.textures.at (textureName);
						}

						textureFile = library.getActualTextureFileName (textureFile);

						const RaylibPropResourceManager::RaylibMeshResource & raylibMeshResource = raylibMeshResources.at (libraryName).at (meshFile);
						const RaylibPropResourceManager::RaylibTextureResource & raylibTextureResource = raylibTextureResources.at (libraryName).at (textureFile);

						sceneMeshes.push_back ({
							.descriptor = {
								.libraryName = libraryName,
								.groupName = groupName,
								.propName = propName,
								.textureName = textureName,
							},
							.position = {
								scale * static_cast <float> (mapObject.positionX),
								scale * static_cast <float> (mapObject.positionY),
								scale * static_cast <float> (mapObject.positionZ),
							},
							.rotation = {
								0,
								0,
								static_cast <float> (mapObject.rotationZ * 180 / std::numbers::pi),
							},
							.scale = {
								scale, scale, scale
							},
							.model = raylibMeshResource.model,
							.texture = raylibTextureResource.texture
						});
					}
				}
				else if (true == group.sprites.contains (propName)) {
					std::string textureFile = library.getActualTextureFileName (group.sprites.at (propName).diffuseFile);
					const RaylibPropResourceManager::RaylibTextureResource & raylibTextureResource = raylibTextureResources.at (libraryName).at (textureFile);
					const RaylibPropResourceManager::RaylibSpriteInfo & spriteInfo = raylibSpriteInfos.at (libraryName).at (textureFile);

					for (const Map::MapObject & prop : propInfo) {
						sceneSprites.push_back ({
							.descriptor = {
								.libraryName = libraryName,
								.groupName = groupName,
								.propName = propName,
							},
							.rect = Rectangle (
								0,
								0,
								raylibTextureResource.texture.width,
								raylibTextureResource.texture.height
							),
							.position = {
								scale * static_cast <float> (prop.positionX),
								scale * static_cast <float> (prop.positionY),
								scale * static_cast <float> (prop.positionZ),
							},
							.size = {
								spriteInfo.size.x * scale,
								spriteInfo.size.y * scale,
							},
							.origin = {
								spriteInfo.origin.x * scale,
								spriteInfo.origin.y * scale,
							},
							.texture = raylibTextureResource.texture,
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
		<< std::chrono::duration <double> (end - start).count ()
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

		for (const SceneMesh & mesh : sceneMeshes) {
			const Model & model = mesh.model;
			model.materials [0].maps [MATERIAL_MAP_DIFFUSE].texture = mesh.texture;

			DrawModelEx (
				model,
				mesh.position,
				{
					0,
					0,
					1,
				},
				mesh.rotation.z,
				mesh.scale,
				mesh.tint
			);
		}

		for (const SceneSprite & sprite : sceneSprites) {
			DrawBillboardPro (
				camera,
				sprite.texture,
				sprite.rect,
				sprite.position,
				{
					0,
					0,
					1,
				},
				sprite.size,
				sprite.origin,
				0,
				sprite.tint
			);
		}

		rlPushMatrix ();
		rlRotatef (90.0f, 1.0f, 0.0f, 0.0f);
		DrawGrid (50, 5.0f);
		rlPopMatrix ();

		EndMode3D ();

		DrawFPS (10, 10);

		EndDrawing ();
	}

	// UnloadTexture (texture);

	CloseWindow ();

	return 0;
}
// NOLINTEND(*)


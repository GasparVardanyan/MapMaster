# include <iostream>
# include <memory>

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
int main(void)
{
	// for (const auto & [groupName, group] : lib.groups ()) {
	// 	for (const auto & [propName, _] : group.meshes) {
	// 		resManager.loadResources(lib.name (), groupName, propName);
	// 	}
	// }

	const int screenWidth = 800;
	const int screenHeight = 450;

	SetConfigFlags (FLAG_MSAA_4X_HINT);
	InitWindow (screenWidth, screenHeight, "raylib [models] example - loading");
	auto start = std::chrono::steady_clock::now ();

	Map map;
	// map.loadFile(DATA_DIR "maps/M/map_silence_remake_cy95v_summer/map.xml");
	map.loadFile(DATA_DIR "maps/Summer/Sandbox_MM.xml");
	// map.loadFile(DATA_DIR "map.xml");

	PropRaylibResourceManager raylibResManager;
	PropResourceManager & m_resourceManager = raylibResManager.resourceManager ();

	for (const auto & [libraryName, groupData] : map.mapObjects ()) {
		raylibResManager.loadLibrary (DATA_DIR "propslibs/" + libraryName);
	}

	raylibResManager.loadMapResources (map);
	// return 0;

	using RaylibMeshResource = PropRaylibResourceManager::RaylibMeshResource;
	using RaylibTextureResource = PropRaylibResourceManager::RaylibTextureResource;
	using RaylibSpriteInfo = PropRaylibResourceManager::RaylibSpriteInfo;

	const std::map <std::string, std::map <std::string, RaylibMeshResource>> & m_meshResources = raylibResManager.meshResources();
	const std::map <std::string, std::map <std::string, RaylibTextureResource>> & m_textureResources = raylibResManager.textureResources();
	const std::map <std::string, std::map <std::string, RaylibSpriteInfo>> & m_spriteInfos = raylibResManager.spriteInfos();


	const auto & libraries = m_resourceManager.propLibraries ();

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
					const PropResourceManager::PropMeshResource & meshResource = const_cast <PropResourceManager::PropMeshResource &> (m_resourceManager.getMeshResource (libraryName, groupName, propName));
					const std::string meshFile = m_resourceManager.propLibraries ().at (libraryName).groups ().at (groupName).meshes.at (propName).file;

					for (const auto & prop : propInfo) {
						std::string textureName = prop.textureName;
						std::string textureFile;
						if (true == textureName.empty ()) {
							textureFile = meshResource.textureFile;
						}
						else {
							textureFile = m_resourceManager.propLibraries ().at (libraryName).groups ().at (groupName).meshes.at (propName).textures.at (textureName);
						}

						textureFile = m_resourceManager.propLibraries ().at (libraryName).actualTextureFile (textureFile);

						sceneMeshes.push_back ({
							.library = libraryName,
							.meshFile = meshFile,
							.textureFile = textureFile,
							.position = {scale * static_cast<float>(prop.positionX), scale * static_cast<float>(prop.positionY), scale * static_cast<float>(prop.positionZ)},
							.rotation = {0, 0, static_cast<float>(prop.rotationZ * 180 / M_PI)}
						});
					}
				}
				else if (true == group.sprites.contains (propName)) {
					const auto & sprite = group.sprites.at (propName);
					std::string textureFile = m_resourceManager.propLibraries ().at (libraryName).actualTextureFile (sprite.diffuseFile);

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

	auto end = std::chrono::steady_clock::now();

	// raylibResManager = {};
	// resManager = {};

	std::cout << "Elapsed: "
		<< std::chrono::duration<double>(end - start).count()
		<< " s\n";


	Camera camera = { 0 };
	camera.position = { 50.0f, 50.0f, 50.0f };
	camera.target = { 0.0f, 12.0f, 0.0f };
	camera.up = { 0.0f, 0.0f, 1.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		UpdateCamera(&camera, CAMERA_THIRD_PERSON);

		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(camera);

		// DrawModel(model, position, 0.05f, WHITE);
		for (const auto & mesh : sceneMeshes) {
			auto & model = m_meshResources.at (mesh.library).at (mesh.meshFile).model;
			auto & texture = m_textureResources.at (mesh.library).at (mesh.textureFile).texture;
			model.materials [0].maps [MATERIAL_MAP_DIFFUSE].texture = texture;
			DrawModelEx(model, mesh.position, {0, 0, 1}, mesh.rotation.z, {scale, scale, scale}, WHITE);
		}

		for (const auto & sprite : sceneSprites) {
			auto & texture = m_textureResources.at (sprite.library).at (sprite.textureFile).texture;
			auto & spdata = m_spriteInfos.at (sprite.library).at (sprite.textureFile);
			// DrawTexturePro(Texture2D texture, Rectangle srcrec, Rectangle dstrec, Vector2 origin, float rotation, Color tint) -> void

			DrawBillboardPro (
				camera,
				m_textureResources.at (sprite.library).at (sprite.textureFile).texture,
				{0, 0, static_cast <float> (texture.width), static_cast <float> (texture.height)},
				sprite.position,
				{0, 0, 1},
				spdata.size,
				spdata.origin,
				0,
				WHITE
			);
			// model.materials [0].maps [MATERIAL_MAP_DIFFUSE].texture = texture;
			// DrawModelEx(model, sprite.position, {0, 0, 1}, sprite.rotation.z * 180 / M_PI, {scale, scale, scale}, WHITE);
		}

		// DrawGrid(20, 10.0f);

		EndMode3D();

		DrawFPS(10, 10);

		EndDrawing();
	}

	// UnloadTexture(texture);

	CloseWindow();

	return 0;
}
// NOLINTEND(*)


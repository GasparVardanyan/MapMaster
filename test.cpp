#include <math.h>
# define SUPPORT_FILEFORMAT_JPG 1

# include "Map.hpp"
# include "PropLibrary.hpp"
#include "PropResourceManager.hpp"

#include <map>
# include <raylib.h>
#include <string>
#include <utility>
#include <vector>

static constexpr float scale = 0.005F;

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

	InitWindow(screenWidth, screenHeight, "raylib [models] example - loading");

		Map map;
		map.loadFile(DATA_DIR "maps/Summer/Sandbox_MM.xml");

		PropResourceManager resManager;
		std::map <std::string, std::map <std::string, std::vector <std::string>>> propsToLoad;

		for (const auto & [libraryName, groups] : map.mapObjects ()) {
			{
				PropLibrary library;
				library.loadDirectory (DATA_DIR "/propslibs/" + libraryName);
				resManager.addPropLibrary (std::move (library));
			}
			for (const auto & [groupName, props] : groups) {
				for (const auto & [propName, propInfo] : props) {
					propsToLoad [libraryName] [groupName].push_back (propName);
					// resManager.loadResources (libraryName, groupName, propName);
				}
			}
		}

		resManager.loadResources (propsToLoad);

		struct RayMesh {
			Mesh mesh;
			Model model;
		};

		struct RayTexture {
			Texture2D texture = {};
		};

		std::map <std::string, std::map <std::string, RayMesh>> meshes;
		std::map <std::string, std::map <std::string, RayTexture>> textures;

		struct SceneObject {
			Vector3 position = {};
			Vector3 rotation = {};
			std::string library;
			std::string meshFile;
			std::string textureFile;
		};

		std::vector <SceneObject> sceneObjects;

		const auto & libraries = resManager.propLibraries ();

		for (const auto & [libraryName, groups] : map.mapObjects ()) {
			const auto & library = libraries.at (libraryName);
			for (const auto & [groupName, props] : groups) {
				const auto & group = library.groups ().at (groupName);
				for (const auto & [propName, propInfo] : props) {
					if (true == group.meshes.contains (propName)) {
						PropResourceManager::PropMeshResource & res = const_cast <PropResourceManager::PropMeshResource &> (resManager.getMeshResource (libraryName, groupName, propName));
						SceneObject obj;
						std::string meshFile = resManager.propLibraries ().at (libraryName).groups ().at (groupName).meshes.at (propName).file;
						if (false == meshes.contains (libraryName) || false == meshes.at (libraryName).contains (meshFile)) {
							RayMesh & m = meshes [libraryName] [meshFile];

							m.mesh.vertices = res.vertexBuffer.data ();
							m.mesh.vertexCount = static_cast <int> (res.vertexBuffer.size () / 3);
							if (false == res.uvBuffer.empty ()) {
								m.mesh.texcoords = res.uvBuffer.data ();
							}
							if (false == res.normalBuffer.empty ()) {
								m.mesh.normals = res.normalBuffer.data ();
							}

							m.mesh.triangleCount = static_cast <int> (res.indexBuffer.size () / 3);

							m.mesh.indices = res.indexBuffer.data();

							UploadMesh (& m.mesh, false);
							m.model = LoadModelFromMesh (m.mesh);
						}
						for (const auto & prop : propInfo) {
							std::string textureName = prop.textureName;
							std::string textureFile;
							if (true == textureName.empty ()) {
								textureFile = res.textureFile;
							}
							else {
								textureFile = resManager.propLibraries ().at (libraryName).groups ().at (groupName).meshes.at (propName).textures.at (textureName);
							}

							textureFile = resManager.propLibraries ().at (libraryName).actualTextureFile (textureFile);

							if (false == textures.contains (libraryName) || false == textures.at (libraryName).contains (textureFile)) {
								std::string texturePath = library.path () + "/" + textureFile;
								textures [libraryName] [textureFile] = {
									.texture = LoadTexture (texturePath.c_str ())
								};
							}

							sceneObjects.push_back ({
								.library = libraryName,
								.meshFile = meshFile,
								.textureFile = textureFile,
								.position = (Vector3){scale * static_cast<float>(prop.positionX), scale * static_cast<float>(prop.positionY), scale * static_cast<float>(prop.positionZ)},
								.rotation = (Vector3){0, 0, static_cast<float>(prop.rotationZ)}
							});
						}

						// if (false == textures.contains (library) || false == textures.at (libraryName).contains ())
					}
				}
			}
		}


	Camera camera = { 0 };
	camera.position = (Vector3){ 50.0f, 50.0f, 50.0f };
	camera.target = (Vector3){ 0.0f, 12.0f, 0.0f };
	camera.up = (Vector3){ 0.0f, 0.0f, 1.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	// PropMesh castle (DATA_DIR "smhouse5.3ds");
	//
	// Model & model = castle.model;
	//
	// Texture2D texture = LoadTexture(DATA_DIR "smhouse5.jpg");
	// model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
	//
	// Vector3 position = { 0.0f, 0.0f, 0.0f };


	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		UpdateCamera(&camera, CAMERA_ORBITAL);

		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(camera);

		// DrawModel(model, position, 0.05f, WHITE);
		for (const auto & obj : sceneObjects) {
			auto & model = meshes.at (obj.library).at (obj.meshFile).model;
			auto & texture = textures.at (obj.library).at (obj.textureFile).texture;
			model.materials [0].maps [MATERIAL_MAP_DIFFUSE].texture = texture;
			DrawModelEx(model, obj.position, {0, 0, 1}, obj.rotation.z * 180 / M_PI, {scale, scale, scale}, WHITE);
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


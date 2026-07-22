# include <iostream>
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
# include "PropResourceManager.hpp"

# include <stb_image.h>


Texture2D LoadTextureStb(const PropResourceManager::PropTextureResource & res) {
	std::cout << "IW: " << res.width << '\n';
	std::cout << "IH: " << res.height << '\n';

	if (res.pixBuffer.get () == nullptr) {
		std::cout << "NPTR\n";
	}

    Image image = {
        .data = static_cast <void *> (res.pixBuffer.get ()),
        .width = res.width,
        .height = res.height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    Texture2D texture = LoadTextureFromImage(image);

	return texture;
}

Texture2D LoadTextureStb(const std::string & diffusePath, const std::string & opacityPath = "")
{
    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char *pixels = stbi_load(
        diffusePath.c_str (),
        &width,
        &height,
        &channels,
        STBI_rgb_alpha
    );

    if (pixels == nullptr)
    {
        TraceLog(LOG_ERROR, "STB: failed to load image %s: %s",
                 diffusePath.c_str (),
                 stbi_failure_reason());

        return {};
    }

    if (false == opacityPath.empty ())
    {
        int alphaWidth = 0;
        int alphaHeight = 0;
        int alphaChannels = 0;
		std::cout << "ALPHA: " << opacityPath << '\n';

        unsigned char *alphaPixels = stbi_load(
            opacityPath.c_str (),
            &alphaWidth,
            &alphaHeight,
            &alphaChannels,
            STBI_grey
        );

        if (alphaPixels == nullptr)
        {
            TraceLog(LOG_WARNING,
                     "STB: failed to load alpha image %s: %s",
                     opacityPath.c_str (),
                     stbi_failure_reason());
        }
        else
        {
            if (alphaWidth != width || alphaHeight != height)
            {
                TraceLog(LOG_WARNING,
                         "STB: alpha image size mismatch (%dx%d vs %dx%d)",
                         alphaWidth, alphaHeight,
                         width, height);
            }
            else
            {
                for (int i = 0; i < width * height; i++)
                {
                    pixels[i * 4 + 3] = alphaPixels[i];
                }
            }

            stbi_image_free(alphaPixels);
        }
    }

    Image image = {
        .data = pixels,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    Texture2D texture = LoadTextureFromImage(image);

    stbi_image_free (pixels);

    return texture;
}







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

	InitWindow(screenWidth, screenHeight, "raylib [models] example - loading");

		auto start = std::chrono::steady_clock::now();



		Map map;
		map.loadFile(DATA_DIR "maps/M/map_silence_remake_cy95v_summer/map.xml");
		// map.loadFile(DATA_DIR "maps/Summer/Sandbox_MM.xml");
		// map.loadFile(DATA_DIR "map.xml");

		PropResourceManager resManager;
		// std::map <std::string, std::map <std::string, std::vector <std::string>>> propsToLoad;

		for (const auto & [libraryName, groups] : map.mapObjects ()) {
			{
				PropLibrary library;
				library.loadDirectory (DATA_DIR "propslibs/" + libraryName);
				// library.loadDirectory (DATA_DIR "/pl2/" + libraryName);
				resManager.addPropLibrary (std::move (library));
			}
			// for (const auto & [groupName, props] : groups) {
			// 	for (const auto & [propName, propInfo] : props) {
			// 		propsToLoad [libraryName] [groupName].push_back (propName);
			// 		// resManager.loadResources (libraryName, groupName, propName);
			// 	}
			// }
		}

		resManager.loadMapResources (map);
		// return 0;

		struct RayMesh {
			Mesh mesh;
			Model model;
		};

		struct RayTexture {
			Texture2D texture = {};
		};

		struct RaySprite {
			PropLibrary::PropSprite::OriginType originX;
			PropLibrary::PropSprite::OriginType originY;
			PropLibrary::PropSprite::ScaleType scale;
		};

		std::map <std::string, std::map <std::string, RayMesh>> meshes;
		std::map <std::string, std::map <std::string, RayTexture>> textures;
		std::map <std::string, std::map <std::string, RaySprite>> sprites;

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
			Vector3 rotation = {};
			std::string library;
			std::string textureFile; // FIXME: use propName
		};

		std::vector <SceneSprite> sceneSprites;

		const auto & libraries = resManager.propLibraries ();

		for (const auto & [libraryName, groups] : map.mapObjects ()) {
			const auto & library = libraries.at (libraryName);
			for (const auto & [groupName, props] : groups) {
				const auto & group = library.groups ().at (groupName);
				for (const auto & [propName, propInfo] : props) {
					if (true == group.meshes.contains (propName)) {
						PropResourceManager::PropMeshResource & meshResource = const_cast <PropResourceManager::PropMeshResource &> (resManager.getMeshResource (libraryName, groupName, propName));
						SceneMesh obj;
						std::string meshFile = resManager.propLibraries ().at (libraryName).groups ().at (groupName).meshes.at (propName).file;
						if (false == meshes.contains (libraryName) || false == meshes.at (libraryName).contains (meshFile)) {
							RayMesh & m = meshes [libraryName] [meshFile];

							m.mesh.vertices = meshResource.vertexBuffer.data ();
							m.mesh.vertexCount = static_cast <int> (meshResource.vertexBuffer.size () / 3);
							if (false == meshResource.uvBuffer.empty ()) {
								m.mesh.texcoords = meshResource.uvBuffer.data ();
							}
							if (false == meshResource.normalBuffer.empty ()) {
								m.mesh.normals = meshResource.normalBuffer.data ();
							}

							m.mesh.triangleCount = static_cast <int> (meshResource.indexBuffer.size () / 3);

							m.mesh.indices = meshResource.indexBuffer.data();

							UploadMesh (& m.mesh, false);
							m.model = LoadModelFromMesh (m.mesh);
						}
						for (const auto & prop : propInfo) {
							std::string textureName = prop.textureName;
							std::string textureFile;
							if (true == textureName.empty ()) {
								textureFile = meshResource.textureFile;
							}
							else {
								textureFile = resManager.propLibraries ().at (libraryName).groups ().at (groupName).meshes.at (propName).textures.at (textureName);
							}

							textureFile = resManager.propLibraries ().at (libraryName).actualTextureFile (textureFile);

							if (false == textures.contains (libraryName) || false == textures.at (libraryName).contains (textureFile)) {
								const PropResourceManager::PropTextureResource & textureResource = resManager.getTextureResource (libraryName, groupName, propName, textureName);
								textures [libraryName] [textureFile] = {
									.texture = LoadTextureStb (textureResource)
								};
							}

							sceneMeshes.push_back ({
								.library = libraryName,
								.meshFile = meshFile,
								.textureFile = textureFile,
								.position = {scale * static_cast<float>(prop.positionX), scale * static_cast<float>(prop.positionY), scale * static_cast<float>(prop.positionZ)},
								.rotation = {0, 0, static_cast<float>(prop.rotationZ)}
							});
						}

						// if (false == textures.contains (library) || false == textures.at (libraryName).contains ())
					}
					else if (true == group.sprites.contains (propName)) {
						const auto & sprite = group.sprites.at (propName);
						std::string textureFile = resManager.propLibraries ().at (libraryName).actualTextureFile (sprite.diffuseFile);
						if (false == textures.contains (libraryName) || false == textures.at (libraryName).contains (textureFile)) {
							auto const & res = resManager.getTextureResource (libraryName, groupName, propName);
							textures [libraryName] [textureFile] = {
								.texture = LoadTextureStb (res)
							};
						}
						if (false == sprites.contains (libraryName) || false == sprites.at (libraryName).contains (textureFile)) { // FIXME: use propName since theoretically multiple sprites can use the same file with different origins and scales
							sprites [libraryName] [textureFile] = {
								.originX = sprite.originX,
								.originY = sprite.originY,
								.scale = sprite.scale
							};
						}

						for (const auto & prop : propInfo) {
							sceneSprites.push_back ({
								.library = libraryName,
								.position = {scale * static_cast<float>(prop.positionX), scale * static_cast<float>(prop.positionY), scale * static_cast<float>(prop.positionZ)},
								.rotation = {0, 0, static_cast<float>(prop.rotationZ)},
								.textureFile = textureFile
							});
						}
					}
				}
			}
		}

		auto end = std::chrono::steady_clock::now();

		std::cout << "Elapsed: "
			<< std::chrono::duration<double>(end - start).count()
			<< " s\n";


	Camera camera = { 0 };
	camera.position = { 50.0f, 50.0f, 50.0f };
	camera.target = { 0.0f, 12.0f, 0.0f };
	camera.up = { 0.0f, 0.0f, 1.0f };
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
		UpdateCamera(&camera, CAMERA_THIRD_PERSON);

		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(camera);

		// DrawModel(model, position, 0.05f, WHITE);
		for (const auto & mesh : sceneMeshes) {
			auto & model = meshes.at (mesh.library).at (mesh.meshFile).model;
			auto & texture = textures.at (mesh.library).at (mesh.textureFile).texture;
			model.materials [0].maps [MATERIAL_MAP_DIFFUSE].texture = texture;
			DrawModelEx(model, mesh.position, {0, 0, 1}, mesh.rotation.z * 180 / M_PI, {scale, scale, scale}, WHITE);
		}

		for (const auto & sprite : sceneSprites) {
			auto & texture = textures.at (sprite.library).at (sprite.textureFile).texture;
			auto & spdata = sprites.at (sprite.library).at (sprite.textureFile);
			// DrawTexturePro(Texture2D texture, Rectangle srcrec, Rectangle dstrec, Vector2 origin, float rotation, Color tint) -> void

			Vector2 size = {static_cast <float> (texture.width * spdata.scale * scale), static_cast <float> (texture.height * spdata.scale * scale)};
			DrawBillboardPro (
				camera,
				textures.at (sprite.library).at (sprite.textureFile).texture,
				{0, 0, static_cast <float> (texture.width), static_cast <float> (texture.height)},
				sprite.position,
				{0, 0, 1},
				size,
				{static_cast <float> (spdata.originX) * size.x, (1 - static_cast <float> (spdata.originY)) * size.y},
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


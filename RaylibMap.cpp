# include "RaylibMap.hpp"
# include "Map.hpp"
# include "PropLibrary.hpp"
# include "PropResourceManager.hpp"
# include "RaylibPropResourceManager.hpp"
# include <map>
# include <memory>
# include <raylib.h>
# include <string>

std::shared_ptr <const RaylibPropResourceManager> RaylibMap::resourceManager () {
	return std::shared_ptr <const RaylibPropResourceManager> (m_raylibResourceManager);
}

void RaylibMap::load (const std::string & mapFile, const std::string & propLibraryDirectory, const float scale) {
	m_sceneObjects = {};
	Map map;
	map.loadFile (mapFile);

	m_raylibResourceManager = std::make_shared <RaylibPropResourceManager> ();

	m_raylibResourceManager->loadMapLibraries (map, propLibraryDirectory);
	m_raylibResourceManager->loadMapResources (map);

	const auto & raylibMeshResources = m_raylibResourceManager->meshResources ();
	const auto & raylibTextureResources = m_raylibResourceManager->textureResources ();
	const auto & raylibSpriteInfos = m_raylibResourceManager->spriteInfos ();

	const PropResourceManager & resourceManager = m_raylibResourceManager->resourceManager ();
	const auto & propLibraries = resourceManager.propLibraries ();

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

						m_sceneObjects.meshes.push_back ({
							.position = {
								.x = scale * static_cast <float> (mapObject.positionX),
								.y = scale * static_cast <float> (mapObject.positionY),
								.z = scale * static_cast <float> (mapObject.positionZ),
							},
							.rotation = {
								.x = 0,
								.y = 0,
								.z = static_cast <float> (mapObject.rotationZ * 180 / std::numbers::pi),
							},
							.scale = {
								.x = scale, .y = scale, .z = scale
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
						m_sceneObjects.sprites.push_back ({
							.rect = {
								.x = 0,
								.y = 0,
								.width = static_cast <float> (raylibTextureResource.texture->width),
								.height = static_cast <float> (raylibTextureResource.texture->height),
							},
							.position = {
								.x = scale * static_cast <float> (prop.positionX),
								.y = scale * static_cast <float> (prop.positionY),
								.z = scale * static_cast <float> (prop.positionZ),
							},
							.size = {
								.x = spriteInfo.size.x * scale,
								.y = spriteInfo.size.y * scale,
							},
							.origin = {
								.x = spriteInfo.origin.x * scale,
								.y = spriteInfo.origin.y * scale,
							},
							.texture = raylibTextureResource.texture,
						});
					}
				}
			}
		}
	}
}

void RaylibMap::render (Camera & camera) {
	for (const SceneMesh & mesh : m_sceneObjects.meshes) {
		const Model & model = * mesh.model;
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		model.materials [0].maps [MATERIAL_MAP_DIFFUSE].texture = * mesh.texture;

		DrawModelEx (
			model,
			mesh.position,
			{ .x = 0, .y = 0, .z = 1 },
			mesh.rotation.z,
			mesh.scale,
			mesh.tint
		);
	}

	for (const SceneSprite & sprite : m_sceneObjects.sprites) {
		DrawBillboardPro (
			camera,
			* sprite.texture,
			sprite.rect,
			sprite.position,
			{ .x = 0, .y = 0, .z = 1 },
			sprite.size,
			sprite.origin,
			0,
			sprite.tint
		);
	}
}

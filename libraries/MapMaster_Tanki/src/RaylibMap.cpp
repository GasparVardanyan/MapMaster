# include "RaylibMap.hpp"

# include <functional>
# include <map>
# include <memory>
# include <numbers>
# include <string>
# include <utility>

# include <raylib.h>
# include <variant>

# include "Map.hpp"
# include "PropLibrary.hpp"
# include "PropResourceManager.hpp"
# include "RaylibPropResourceManager.hpp"

void RaylibMap::loadScene (float scale) {
	m_sceneObjects = {};

	const auto & raylibMeshResources = m_raylibResourceManager->meshResources ();
	const auto & raylibMultiMeshResources = m_raylibResourceManager->multiMeshResources ();
	const auto & raylibTextureResources = m_raylibResourceManager->textureResources ();
	const auto & raylibSpriteInfos = m_raylibResourceManager->spriteInfos ();

	// cppcheck-suppress shadowFunction
	const PropResourceManager & resourceManager = m_raylibResourceManager->resourceManager ();
	const auto & propLibraries = resourceManager.propLibraries ();

	for (const auto & [libraryName, groups] : m_map->mapObjects ()) {
		const PropLibrary & library = * propLibraries.at (libraryName);
		const std::map <std::string, PropLibrary::Group> & libraryGroups = library.groups ();

		for (const auto & [groupName, props] : groups) {
			const PropLibrary::Group & group = libraryGroups.at (groupName);

			for (const auto & [propName, propInfo] : props) {
				if (true == group.meshes.contains (propName)) {

					auto meshVariant = resourceManager.getMeshOrMultiMeshResource (libraryName, groupName, propName);

					if (true == std::holds_alternative <std::reference_wrapper <const PropResourceManager::PropMeshResource>> (meshVariant)) {
						const PropResourceManager::PropMeshResource & meshResource = const_cast <PropResourceManager::PropMeshResource &> (
							std::get <std::reference_wrapper <const PropResourceManager::PropMeshResource>> (meshVariant).get ()
						);
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
									// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
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
					else {
						const PropLibrary::PropMesh & propMesh = group.meshes.at (propName);
						const std::string meshFile = propMesh.file;

						for (const Map::MapObject & mapObject : propInfo) {
							const RaylibPropResourceManager::RaylibMultiMeshResource & raylibMeshResource = raylibMultiMeshResources.at (libraryName).at (meshFile);

							m_sceneObjects.meshes.push_back ({
								.position = {
									.x = scale * static_cast <float> (mapObject.positionX),
									.y = scale * static_cast <float> (mapObject.positionY),
									.z = scale * static_cast <float> (mapObject.positionZ),
								},
								.rotation = {
									.x = 0,
									.y = 0,
									// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
									.z = static_cast <float> (mapObject.rotationZ * 180 / std::numbers::pi),
								},
								.scale = {
									.x = scale, .y = scale, .z = scale
								},
								.model = raylibMeshResource.model,
								.texture = nullptr,
							});
						}
					}
				}
				else if (true == group.sprites.contains (propName)) {
					std::string textureFile = library.getActualTextureFileName (group.sprites.at (propName).diffuseFile);
					const RaylibPropResourceManager::RaylibTextureResource & raylibTextureResource = raylibTextureResources.at (libraryName).at (textureFile);
					const RaylibPropResourceManager::RaylibSpriteInfo & spriteInfo = raylibSpriteInfos.at (libraryName).at (textureFile);

					for (const Map::MapObject & prop : propInfo) {
						m_sceneObjects.sprites.push_back ({
							.position = {
								.x = scale * static_cast <float> (prop.positionX),
								.y = scale * static_cast <float> (prop.positionY),
								.z = scale * static_cast <float> (prop.positionZ),
							},
							.size = {
								.x = spriteInfo.size.x * scale,
								.y = spriteInfo.size.y * scale,
							},
							.rect = {
								.x = 0,
								.y = 0,
								.width = static_cast <float> (raylibTextureResource.texture->width),
								.height = static_cast <float> (raylibTextureResource.texture->height),
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
		if (nullptr != mesh.texture) {
			model.materials [0].maps [MATERIAL_MAP_DIFFUSE].texture = * mesh.texture;
		}

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

// cppcheck-suppress shadowFunction
void RaylibMap::setResourceManager (std::shared_ptr <RaylibPropResourceManager> resourceManager) {
	m_raylibResourceManager = std::move (resourceManager);
}

std::shared_ptr <RaylibPropResourceManager> RaylibMap::resourceManager () const {
	return m_raylibResourceManager;
}

// cppcheck-suppress shadowFunction
void RaylibMap::setMap (std::shared_ptr <Map> map) {
	m_map = std::move (map);
}

std::shared_ptr <Map> RaylibMap::map () const {
	return m_map;
}

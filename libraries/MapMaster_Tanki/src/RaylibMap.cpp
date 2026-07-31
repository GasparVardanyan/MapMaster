# include "RaylibMap.hpp"

# include <map>
# include <memory>
# include <numbers>
# include <string>
# include <utility>

# include <Eigen/Core>
# include <Eigen/Geometry>
# include <raylib.h>
# include <rlgl.h>

# include "Map.hpp"
# include "PropLibrary.hpp"
# include "PropResourceManager.hpp"
# include "RaylibPropResourceManager.hpp"

void RaylibMap::loadScene (float scale) {
	m_sceneObjects = {};

	const auto & raylibMeshResources = m_raylibResourceManager->meshResources ();
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

					const PropResourceManager::PropMeshResource & meshResource = const_cast <PropResourceManager::PropMeshResource &> (
						resourceManager.getMeshResource (libraryName, groupName, propName)
					);
					const PropLibrary::PropMesh & propMesh = group.meshes.at (propName);
					const std::string meshFile = propMesh.file;

					const PropResourceManager::Collider & collider = * resourceManager.colliders ().at (libraryName).at (meshFile);


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
							.texture = raylibTextureResource.texture,
						});

						Eigen::Affine3d transform = Eigen::Affine3d::Identity ();
						transform.translate (Eigen::Vector3d (
							mapObject.positionX,
							mapObject.positionY,
							mapObject.positionZ
						));

						transform.rotate (Eigen::AngleAxisd (
							mapObject.rotationZ,
							Eigen::Vector3d ::UnitZ ()
						));

						// transform.scale (scale);

						for (const PropResourceManager::Collider::TriangleCollider & triangleCollider : collider.triangleColliders) {
							Eigen::Vector3d v1 = transform * Eigen::Vector3d (triangleCollider.v1.x, triangleCollider.v1.y, triangleCollider.v1.z);
							Eigen::Vector3d v2 = transform * Eigen::Vector3d (triangleCollider.v2.x, triangleCollider.v2.y, triangleCollider.v2.z);
							Eigen::Vector3d v3 = transform * Eigen::Vector3d (triangleCollider.v3.x, triangleCollider.v3.y, triangleCollider.v3.z);

							m_sceneObjects.triangleColliders.push_back ({
								.v1 = { .x = scale * static_cast <float> (v1.x ()), .y = scale * static_cast <float> (v1.y ()), .z = scale * static_cast <float> (v1.z ())},
								.v2 = { .x = scale * static_cast <float> (v2.x ()), .y = scale * static_cast <float> (v2.y ()), .z = scale * static_cast <float> (v2.z ())},
								.v3 = { .x = scale * static_cast <float> (v3.x ()), .y = scale * static_cast <float> (v3.y ()), .z = scale * static_cast <float> (v3.z ())},
							});
						}

						for (const PropResourceManager::Collider::RectCollider & rectCollider : collider.rectColliders) {
							Eigen::Vector3d v1 = transform * Eigen::Vector3d (rectCollider.v1.x, rectCollider.v1.y, rectCollider.v1.z);
							Eigen::Vector3d v2 = transform * Eigen::Vector3d (rectCollider.v2.x, rectCollider.v2.y, rectCollider.v2.z);
							Eigen::Vector3d v3 = transform * Eigen::Vector3d (rectCollider.v3.x, rectCollider.v3.y, rectCollider.v3.z);
							Eigen::Vector3d v4 = transform * Eigen::Vector3d (rectCollider.v4.x, rectCollider.v4.y, rectCollider.v4.z);

							m_sceneObjects.rectColliders.push_back ({
								.v1 = { .x = scale * static_cast <float> (v1.x ()), .y = scale * static_cast <float> (v1.y ()), .z = scale * static_cast <float> (v1.z ())},
								.v2 = { .x = scale * static_cast <float> (v2.x ()), .y = scale * static_cast <float> (v2.y ()), .z = scale * static_cast <float> (v2.z ())},
								.v3 = { .x = scale * static_cast <float> (v3.x ()), .y = scale * static_cast <float> (v3.y ()), .z = scale * static_cast <float> (v3.z ())},
								.v4 = { .x = scale * static_cast <float> (v4.x ()), .y = scale * static_cast <float> (v4.y ()), .z = scale * static_cast <float> (v4.z ())},
							});
						}

						for (const PropResourceManager::Collider::BoxCollider & boxCollider : collider.boxColliders) {
							Eigen::Vector3d v1 = transform * Eigen::Vector3d (boxCollider.vMin.x, boxCollider.vMin.y, boxCollider.vMin.z);
							Eigen::Vector3d v2 = transform * Eigen::Vector3d (boxCollider.vMax.x, boxCollider.vMax.y, boxCollider.vMax.z);

							Eigen::Vector3d vMin, vMax;

							if (v1.x () < v2.x ()) {
								vMin.x () = v1.x ();
								vMax.x () = v2.x ();
							}
							else {
								vMin.x () = v2.x ();
								vMax.x () = v1.x ();
							}

							if (v1.y () < v2.y ()) {
								vMin.y () = v1.y ();
								vMax.y () = v2.y ();
							}
							else {
								vMin.y () = v2.y ();
								vMax.y () = v1.y ();
							}

							if (v1.z () < v2.z ()) {
								vMin.z () = v1.z ();
								vMax.z () = v2.z ();
							}
							else {
								vMin.z () = v2.z ();
								vMax.z () = v1.z ();
							}

							Eigen::Vector3d position = (vMin + vMax) / 2;
							Eigen::Vector3d size = vMax - vMin;

							m_sceneObjects.boxColliders.push_back ({
								.position = { .x = scale * static_cast <float> (position.x ()), .y = scale * static_cast <float> (position.y ()), .z = scale * static_cast <float> (position.z ())},
								.size = { .x = scale * static_cast <float> (size.x ()), .y = scale * static_cast <float> (size.y ()), .z = scale * static_cast <float> (size.z ())},
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

void RaylibMap::renderCollisionGeometry (bool wireframe) {
	for (const SceneTriangleCollider & collider : m_sceneObjects.triangleColliders) {
		DrawTriangle3D (collider.v1, collider.v2, collider.v3, {.r = 0xFF, .g = 0x7F, .b = 0x7F, .a = 0xFF});
		DrawTriangle3D (collider.v1, collider.v3, collider.v2, {.r = 0xFF, .g = 0x7F, .b = 0x7F, .a = 0xFF});
	}

	for (const SceneRectCollider & collider : m_sceneObjects.rectColliders) {
		DrawTriangle3D (collider.v1, collider.v2, collider.v3, {.r = 0xFF, .g = 0x7F, .b = 0x7F, .a = 0xFF});
		DrawTriangle3D (collider.v1, collider.v3, collider.v2, {.r = 0xFF, .g = 0x7F, .b = 0x7F, .a = 0xFF});

		DrawTriangle3D (collider.v2, collider.v3, collider.v4, {.r = 0xFF, .g = 0x7F, .b = 0x7F, .a = 0xFF});
		DrawTriangle3D (collider.v2, collider.v4, collider.v3, {.r = 0xFF, .g = 0x7F, .b = 0x7F, .a = 0xFF});
	}

	for (const SceneBoxCollider & collider : m_sceneObjects.boxColliders) {
		DrawCubeV (collider.position, collider.size, {.r = 0xFF, .g = 0x7F, .b = 0x7F, .a = 0xFF});
	}

	if (true == wireframe) {
		rlEnableWireMode ();
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
		rlDisableWireMode ();
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

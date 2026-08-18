# include "MapMaster/Tanki/MapRendererRaylibBackend.hpp"

# include <map>
# include <memory>
# include <string>
# include <utility>

# include <raylib.h>
# include <raymath.h>

# include "MapMaster/Tanki/Map.hpp"
# include "MapMaster/Tanki/PropLibrary.hpp"
# include "MapMaster/Tanki/PropGPUResourceManager.hpp"

using namespace MapMaster::Tanki;

void MapRendererRaylibBackend::loadScene (float scale) {
	m_sceneObjects = {};

	const auto & raylibMeshResources = m_raylibResourceManager->meshResources ();
	const auto & raylibTextureResources = m_raylibResourceManager->textureResources ();
	const auto & raylibSpriteInfos = m_raylibResourceManager->spriteInfos ();

	const CPUResourceManager & resourceManager = m_raylibResourceManager->cpuResourceManager ();
	const auto & propLibraries = resourceManager.propLibraries ();

	for (const auto & [libraryName, groups] : m_map->mapObjects ()) {
		const PropLibrary & library = * propLibraries.at (libraryName);
		const std::map <std::string, PropLibrary::Group> & libraryGroups = library.groups ();

		for (const auto & [groupName, props] : groups) {
			const PropLibrary::Group & group = libraryGroups.at (groupName);

			for (const auto & [propName, propInfo] : props) {
				if (true == group.meshes.contains (propName)) {
					const typename CPUResourceManager::PropMeshResource & meshResource = const_cast <CPUResourceManager::PropMeshResource &> (
						resourceManager.getMeshResource (libraryName, groupName, propName)
					);

					const PropLibrary::PropMesh & propMesh = group.meshes.at (propName);
					const std::string meshFile = propMesh.file;

					const typename CPUResourceManager::Collider & collider = * resourceManager.colliders ().at (libraryName).at (meshFile);

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

						const typename GPUResourceManager::MeshResource & raylibMeshResource = raylibMeshResources.at (libraryName).at (meshFile);
						const typename GPUResourceManager::TextureResource & raylibTextureResource = raylibTextureResources.at (libraryName).at (textureFile);

						const Matrix transform = MatrixMultiply (
							MatrixMultiply (
								MatrixRotateZ (
									// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
									static_cast <float> (mapObject.rotationZ)
								),
								MatrixTranslate (
									static_cast <float> (mapObject.positionX),
									static_cast <float> (mapObject.positionY),
									static_cast <float> (mapObject.positionZ)
								)
							),
							MatrixScale (scale, scale, scale)
						);

						SceneMesh sceneMesh = {
							.transform = transform,
							.mesh = raylibMeshResource.mesh,
							.material = LoadMaterialDefault (),
						};
						// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
						sceneMesh.material.maps [MATERIAL_MAP_DIFFUSE].texture = * raylibTextureResource.texture;

						for (const typename CPUResourceManager::Collider::TriangleCollider & triangleCollider : collider.triangleColliders) {
							sceneMesh.triangleColliders.push_back ({
								.v1 = Vector3Transform (Vector3 { .x = triangleCollider.v1.x, .y = triangleCollider.v1.y, .z = triangleCollider.v1.z}, transform),
								.v2 = Vector3Transform (Vector3 { .x = triangleCollider.v2.x, .y = triangleCollider.v2.y, .z = triangleCollider.v2.z}, transform),
								.v3 = Vector3Transform (Vector3 { .x = triangleCollider.v3.x, .y = triangleCollider.v3.y, .z = triangleCollider.v3.z}, transform),
							});
						}

						for (const typename CPUResourceManager::Collider::RectCollider & rectCollider : collider.rectColliders) {
							sceneMesh.rectColliders.push_back ({
								.v1 = Vector3Transform (Vector3 { .x = rectCollider.v1.x, .y = rectCollider.v1.y, .z = rectCollider.v1.z}, transform),
								.v2 = Vector3Transform (Vector3 { .x = rectCollider.v2.x, .y = rectCollider.v2.y, .z = rectCollider.v2.z}, transform),
								.v3 = Vector3Transform (Vector3 { .x = rectCollider.v3.x, .y = rectCollider.v3.y, .z = rectCollider.v3.z}, transform),
								.v4 = Vector3Transform (Vector3 { .x = rectCollider.v4.x, .y = rectCollider.v4.y, .z = rectCollider.v4.z}, transform),
							});
						}

						for (const typename CPUResourceManager::Collider::BoxCollider & boxCollider : collider.boxColliders) {
							Vector3 v1 = Vector3Transform (Vector3 { .x = boxCollider.vMin.x, .y = boxCollider.vMin.y, .z = boxCollider.vMin.z}, transform);
							Vector3 v2 = Vector3Transform (Vector3 { .x = boxCollider.vMax.x, .y = boxCollider.vMax.y, .z = boxCollider.vMax.z}, transform);
							Vector3 vMin, vMax;

							if (v1.x < v2.x) {
								vMin.x = v1.x;
								vMax.x = v2.x;
							}
							else {
								vMin.x = v2.x;
								vMax.x = v1.x;
							}

							if (v1.y < v2.y) {
								vMin.y = v1.y;
								vMax.y = v2.y;
							}
							else {
								vMin.y = v2.y;
								vMax.y = v1.y;
							}

							if (v1.z < v2.z) {
								vMin.z = v1.z;
								vMax.z = v2.z;
							}
							else {
								vMin.z = v2.z;
								vMax.z = v1.z;
							}

							sceneMesh.boxColliders.push_back ({
								// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
								.position = Vector3Scale (Vector3Add (vMin, vMax), 0.5F),
								.size = Vector3Subtract (vMax, vMin),
							});
						}

						m_sceneObjects.meshes.push_back (std::move (sceneMesh));
					}
				}
				else if (true == group.sprites.contains (propName)) {
					std::string textureFile = library.getActualTextureFileName (group.sprites.at (propName).diffuseFile);
					const typename GPUResourceManager::TextureResource & raylibTextureResource = raylibTextureResources.at (libraryName).at (textureFile);
					const typename GPUResourceManager::SpriteInfo & spriteInfo = raylibSpriteInfos.at (libraryName).at (textureFile);

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

void MapRendererRaylibBackend::render (Camera & camera) {
	for (const SceneMesh & mesh : m_sceneObjects.meshes) {
		DrawMesh (* mesh.mesh, mesh.material, mesh.transform);
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

void MapRendererRaylibBackend::renderCollisionGeometry (bool wireframe) {
	for (const SceneMesh & mesh : m_sceneObjects.meshes) {
		for (const SceneTriangleCollider & collider : mesh.triangleColliders) {
			DrawTriangle3D (collider.v1, collider.v2, collider.v3, m_collisionGeometryFaceColor);
			DrawTriangle3D (collider.v1, collider.v3, collider.v2, m_collisionGeometryFaceColor);

			if (true == wireframe) {
				DrawLine3D (collider.v1, collider.v2, m_collisionGeometryEdgeColor);
				DrawLine3D (collider.v2, collider.v3, m_collisionGeometryEdgeColor);
				DrawLine3D (collider.v3, collider.v1, m_collisionGeometryEdgeColor);
			}
		}

		for (const SceneRectCollider & collider : mesh.rectColliders) {
			DrawTriangle3D (collider.v1, collider.v2, collider.v3, m_collisionGeometryFaceColor);
			DrawTriangle3D (collider.v1, collider.v3, collider.v2, m_collisionGeometryFaceColor);

			DrawTriangle3D (collider.v2, collider.v3, collider.v4, m_collisionGeometryFaceColor);
			DrawTriangle3D (collider.v2, collider.v4, collider.v3, m_collisionGeometryFaceColor);

			if (true == wireframe) {
				DrawLine3D (collider.v1, collider.v2, m_collisionGeometryEdgeColor);
				DrawLine3D (collider.v2, collider.v4, m_collisionGeometryEdgeColor);
				DrawLine3D (collider.v4, collider.v3, m_collisionGeometryEdgeColor);
				DrawLine3D (collider.v3, collider.v1, m_collisionGeometryEdgeColor);
			}
		}

		for (const SceneBoxCollider & collider : mesh.boxColliders) {
			DrawCubeV (collider.position, collider.size, m_collisionGeometryFaceColor);

			if (true == wireframe) {
				DrawCubeWiresV (collider.position, collider.size, m_collisionGeometryEdgeColor);
			}
		}
	}
}

// cppcheck-suppress shadowFunction
void MapRendererRaylibBackend::setResourceManager (std::shared_ptr <GPUResourceManager> resourceManager) {
	m_raylibResourceManager = std::move (resourceManager);
}

std::shared_ptr <MapRendererRaylibBackend::GPUResourceManager> MapRendererRaylibBackend::resourceManager () const {
	return m_raylibResourceManager;
}

// cppcheck-suppress shadowFunction
void MapRendererRaylibBackend::setMap (std::shared_ptr <Map> map) {
	m_map = std::move (map);
}

std::shared_ptr <Map> MapRendererRaylibBackend::map () const {
	return m_map;
}

void MapRendererRaylibBackend::setCollisionGeometryFaceColor (Color color) {
	m_collisionGeometryFaceColor = color;
}

void MapRendererRaylibBackend::setCollisionGeometryEdgeColor (Color color) {
	m_collisionGeometryEdgeColor = color;
}

Color MapRendererRaylibBackend::collisionGeometryFaceColor () {
	return m_collisionGeometryFaceColor;
}

Color MapRendererRaylibBackend::collisionGeometryEdgeColor () {
	return m_collisionGeometryEdgeColor;
}

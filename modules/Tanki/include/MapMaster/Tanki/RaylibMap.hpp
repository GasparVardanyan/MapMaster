# pragma once

# include <cstddef>
# include <memory>
# include <vector>

# include <raylib.h>

# include "MapMaster/Tanki/Map.hpp"
# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/RaylibPropResourceManager.hpp"

namespace MapMaster::Tanki {

// cppcheck-suppress-begin unusedStructMember
class RaylibMap {
public:
	struct SceneMesh {
		Matrix transform = {};
		std::shared_ptr <Mesh> mesh = nullptr;
		Material material = {};
	};

	struct SceneSprite {
		Vector3 position = {};
		Vector2 size = {};
		Rectangle rect = {};
		Vector2 origin = {};
		std::shared_ptr <Texture2D> texture = nullptr;
		Color tint = WHITE;
	};

	struct SceneTriangleCollider {
		Vector3 v1 = {};
		Vector3 v2 = {};
		Vector3 v3 = {};
	};

	struct SceneRectCollider {
		Vector3 v1 = {};
		Vector3 v2 = {};
		Vector3 v3 = {};
		Vector3 v4 = {};
	};

	struct SceneBoxCollider {
		Vector3 position = {};
		Vector3 size = {};
	};

	[[nodiscard]] std::shared_ptr <RaylibPropResourceManager> resourceManager () const;
	void setResourceManager (std::shared_ptr <RaylibPropResourceManager> resourceManager);

	[[nodiscard]] std::shared_ptr <Map> map () const;
	void setMap (std::shared_ptr <Map> map);

	void loadScene (float scale);

	void render (Camera & camera);
	void renderCollisionGeometry (bool wireframe = true);

	void setCollisionGeometryFaceColor (Color color);
	void setCollisionGeometryEdgeColor (Color color);
	Color collisionGeometryFaceColor ();
	Color collisionGeometryEdgeColor ();

private:
	std::shared_ptr <RaylibPropResourceManager> m_raylibResourceManager = std::make_shared <RaylibPropResourceManager> (false);
	std::shared_ptr <Map> m_map = std::make_shared <Map> ();

	struct {
		std::vector <SceneMesh> meshes;
		std::vector <SceneSprite> sprites;
		std::vector <SceneTriangleCollider> triangleColliders;
		std::vector <SceneRectCollider> rectColliders;
		std::vector <SceneBoxCollider> boxColliders;
	} m_sceneObjects;

	struct {
		std::vector <std::size_t> meshToBoxColliderMap;
		std::vector <std::size_t> meshToRectColliderMap;
		std::vector <std::size_t> meshToTriangleColliderMap;

		std::vector <std::size_t> meshToBoxColliderCountMap;
		std::vector <std::size_t> meshToRectColliderCountMap;
		std::vector <std::size_t> meshToTriangleColliderCountMap;

		std::vector <std::size_t> boxColliderToMeshMap;
		std::vector <std::size_t> rectColliderToMeshMap;
		std::vector <std::size_t> triangleColliderToMeshMap;
	} m_indexMaps;

	// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	Color m_collisionGeometryFaceColor = {.r = 0xFF, .g = 0x7F, .b = 0x7F, .a = 0xFF};
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	Color m_collisionGeometryEdgeColor = {.r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF};
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

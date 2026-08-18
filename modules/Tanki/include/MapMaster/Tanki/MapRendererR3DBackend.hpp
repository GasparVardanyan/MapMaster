# pragma once

# include <cstddef>
# include <memory>
# include <vector>

# include <raylib.h>

# include "MapMaster/Tanki/Map.hpp"
# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerRaylibBackend.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerR3DBackend.hpp"
# include "MapMaster/Tanki/PropGPUResourceManager.hpp"
# include "MapMaster/Tanki/PropGPUResourceManagerRaylibBackend.hpp"
# include "MapMaster/Tanki/PropGPUResourceManagerR3DBackend.hpp"

# include "r3d/r3d_material.h"

namespace MapMaster::Tanki {

// cppcheck-suppress-begin unusedStructMember
class MapRendererR3DBackend {
public:
	using CPUResourceManager = PropCPUResourceManager <PropCPUResourceManagerR3DBackend>;
	using GPUResourceManager = PropGPUResourceManager <PropGPUResourceManagerR3DBackend>;

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

	struct SceneMesh {
		Matrix transform = {};
		std::shared_ptr <R3D_Mesh> mesh = nullptr;
		R3D_Material material = {};

		std::vector <SceneTriangleCollider> triangleColliders;
		std::vector <SceneRectCollider> rectColliders;
		std::vector <SceneBoxCollider> boxColliders;
	};

	struct SceneSprite {
		Vector3 position = {};
		Vector2 size = {};
		Rectangle rect = {};
		Vector2 origin = {};
		std::shared_ptr <Texture2D> texture = nullptr;
		Color tint = WHITE;
	};
	[[nodiscard]] std::shared_ptr <GPUResourceManager> resourceManager () const;
	void setResourceManager (std::shared_ptr <GPUResourceManager> resourceManager);

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
	std::shared_ptr <GPUResourceManager> m_raylibResourceManager = std::make_shared <GPUResourceManager> (false);
	std::shared_ptr <Map> m_map = std::make_shared <Map> ();

	struct {
		std::vector <SceneMesh> meshes;
		std::vector <SceneSprite> sprites;
	} m_sceneObjects;

	// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	Color m_collisionGeometryFaceColor = {.r = 0xFF, .g = 0x7F, .b = 0x7F, .a = 0xFF};
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	Color m_collisionGeometryEdgeColor = {.r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF};
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

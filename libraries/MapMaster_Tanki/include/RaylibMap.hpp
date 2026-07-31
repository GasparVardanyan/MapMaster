# pragma once

# include <memory>
# include <vector>

# include <raylib.h>

# include "Map.hpp"
# include "PropResourceManager.hpp"
# include "RaylibPropResourceManager.hpp"

// cppcheck-suppress-begin unusedStructMember
class RaylibMap {
public:
	struct SceneMesh {
		Vector3 position = {};
		Vector3 rotation = {};
		Vector3 scale = {};
		std::shared_ptr <Model> model = nullptr;
		std::shared_ptr <Texture2D> texture = nullptr;
		Color tint = WHITE;
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
};
// cppcheck-suppress-end unusedStructMember

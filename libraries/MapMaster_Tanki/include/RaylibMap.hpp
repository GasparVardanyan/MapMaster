# pragma once

# include <memory>
# include <raylib.h>
# include <vector>

# include "Map.hpp"
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

	[[nodiscard]] std::shared_ptr <RaylibPropResourceManager> resourceManager () const;
	void setResourceManager (std::shared_ptr <RaylibPropResourceManager> resourceManager);

	[[nodiscard]] std::shared_ptr <Map> map () const;
	void setMap (std::shared_ptr <Map> map);

	void loadScene (float scale);

	void render (Camera & camera);

private:
	std::shared_ptr <RaylibPropResourceManager> m_raylibResourceManager = std::make_shared <RaylibPropResourceManager> ();
	std::shared_ptr <Map> m_map = std::make_shared <Map> ();
	struct {
		std::vector <SceneMesh> meshes;
		std::vector <SceneSprite> sprites;
	} m_sceneObjects;
};
// cppcheck-suppress-end unusedStructMember

# pragma once

# include "RaylibPropResourceManager.hpp"
# include <memory>
# include <raylib.h>
# include <vector>

class RaylibMap {
public:
	struct SceneMesh {
		Vector3 position = {};
		Vector3 rotation = {};
		Vector3 scale = {};
		// cppcheck-suppress uninitMemberVarNoCtor
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
		std::shared_ptr <Model> model;
		// cppcheck-suppress uninitMemberVarNoCtor
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
		std::shared_ptr <Texture2D> texture;
		Color tint = WHITE;
	};

	struct SceneSprite {
		Vector3 position = {};
		Rectangle rect = {};
		Vector2 size = {};
		Vector2 origin = {};
		// cppcheck-suppress uninitMemberVarNoCtor
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
		std::shared_ptr <Texture2D> texture;
		Color tint = WHITE;
	};

	std::shared_ptr <const RaylibPropResourceManager> resourceManager ();

private:
	std::shared_ptr <RaylibPropResourceManager> m_raylibResourceManager;
	struct {
		std::vector <SceneMesh> meshes;
		std::vector <SceneSprite> sprites;
	} m_sceneObjects;
};

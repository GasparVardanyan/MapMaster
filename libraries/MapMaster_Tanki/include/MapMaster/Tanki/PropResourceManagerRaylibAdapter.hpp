# pragma once

# include <memory>

# include <raylib.h>

# include "MapMaster/Tanki/PropResourceManager.hpp"

// cppcheck-suppress-begin unusedStructMember
class PropResourceManagerRaylibAdapter {
public:
	struct MeshResource {
		std::shared_ptr <Mesh> mesh;

		// TODO: clone()
	};

	struct TextureResource {
		Image image;
		std::shared_ptr <Texture2D> texture;

		// TODO: clone()
	};

	struct SpriteInfo {
		Vector2 origin;
		Vector2 size;
	};

	static MeshResource loadMeshResource (PropResourceManager::PropMeshResource & meshResource);
	static TextureResource loadTextureResource (const PropResourceManager::PropTextureResource & textureResource);

private:
	static void unloadMeshResource (Mesh * mesh);
	static void unloadTextureResource (Texture2D * texture);
};
// cppcheck-suppress-end unusedStructMember

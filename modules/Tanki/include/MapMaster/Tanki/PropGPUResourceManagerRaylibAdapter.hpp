# pragma once

# include <memory>

# include <raylib.h>

# include "MapMaster/Tanki/PropLibrary.hpp"
# include "MapMaster/Tanki/PropCPUResourceManager.hpp"

namespace MapMaster::Tanki {

// TODO: make an interface trait
// cppcheck-suppress-begin unusedStructMember
class PropGPUResourceManagerRaylibAdapter {
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

	static MeshResource LoadMeshResource (PropCPUResourceManager::PropMeshResource & meshResource);
	static TextureResource LoadTextureResource (const PropCPUResourceManager::PropTextureResource & textureResource);
	static SpriteInfo MakeSpriteInfo (const PropLibrary::PropSprite & spriteInfo, const PropCPUResourceManager::PropTextureResource & textureResource);

private:
	static void unloadMeshResource (Mesh * mesh);
	static void unloadTextureResource (Texture2D * texture);
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

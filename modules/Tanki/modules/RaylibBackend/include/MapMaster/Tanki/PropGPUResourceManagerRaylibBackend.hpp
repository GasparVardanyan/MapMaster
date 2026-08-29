# pragma once

# include <memory>

# include <raylib.h>

# include "MapMaster/Tanki/PrimitiveFactory.hpp"
# include "MapMaster/Tanki/PrimitiveFactoryRaylibBackend.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerRaylibBackend.hpp"
# include "MapMaster/Tanki/PropLibrary.hpp"
# include "MapMaster/Tanki/PropCPUResourceManager.hpp"

namespace MapMaster::Tanki {

// cppcheck-suppress-begin unusedStructMember
class PropGPUResourceManagerRaylibBackend {
public:
	using CPUResourceManagerBackend = PropCPUResourceManagerRaylibBackend;
	using CPUResourceManager = PropCPUResourceManager <CPUResourceManagerBackend>;
	using PrimitiveFactoryBackend = PrimitiveFactoryRaylibBackend;
	using PrimitiveFactory = PrimitiveFactory <PrimitiveFactoryBackend>;

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

	template <bool Upload = true>
	static MeshResource CreateMeshResource (CPUResourceManager::PropMeshResource & meshResource);

	template <bool Upload = true>
	static TextureResource CreateTextureResource (const CPUResourceManager::PropTextureResource & textureResource);

	static void UploadMeshResource (MeshResource & meshResource);
	static void UploadTextureResource (TextureResource & textureResource);

	static SpriteInfo CreateSpriteInfo (const PropLibrary::PropSprite & spriteInfo, const CPUResourceManager::PropTextureResource & textureResource);

private:
	static void unloadMeshResource (Mesh * mesh);
	static void unloadTextureResource (Texture2D * texture);
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

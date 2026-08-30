# pragma once

# include <memory>

# include <raylib.h>

# include "MapMaster/Tanki/PrimitiveFactory.hpp"
# include "MapMaster/Tanki/PrimitiveFactoryRaylibBackend.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerRaylibBackend.hpp"
# include "MapMaster/Tanki/PropLibrary.hpp"
# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/PropMetaData.hpp"

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
		PropMetaData::Mesh meta;
	};

	struct TextureResource {
		std::shared_ptr <Texture2D> texture;
		PropMetaData::Texture meta;
	};

	struct SpriteInfo {
		Vector2 origin;
		Vector2 size;
	};

	static MeshResource CreateMeshResource (CPUResourceManager::PropMeshResource & meshResource);
	static TextureResource CreateTextureResource (const CPUResourceManager::PropTextureResource & textureResource);

	static SpriteInfo CreateSpriteInfo (const PropLibrary::PropSprite & sprite, const PropMetaData::Texture & meta);

private:
	static void unloadMeshResource (Mesh * mesh);
	static void unloadTextureResource (Texture2D * texture);
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

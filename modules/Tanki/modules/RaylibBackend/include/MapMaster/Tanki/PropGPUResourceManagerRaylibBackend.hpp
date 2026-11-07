# pragma once

# include <memory>

# include <raylib.h>

# include "MapMaster/Tanki/PrimitiveFactory.hpp"
# include "MapMaster/Tanki/PrimitiveFactoryRaylibBackend.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerRaylibBackend.hpp"
# include "MapMaster/Tanki/PropLibrary.hpp"
# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/PropMetaData.hpp"

namespace MapMaster::Tanki::PropMetaData { struct Mesh; struct Texture; }



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
		std::shared_ptr <PropMetaData::Mesh> meta;
	};

	struct TextureResource {
		std::shared_ptr <Texture2D> texture;
		std::shared_ptr <PropMetaData::Texture> meta;
	};

	struct SpriteResource {
		Vector2 size;
		Vector2 origin;
	};

	static MeshResource CreateMeshResource (CPUResourceManager::PropMeshResource & meshResource);
	static TextureResource CreateTextureResource (const CPUResourceManager::PropTextureResource & textureResource);
	static SpriteResource CreateSpriteResource (const CPUResourceManager::PropSpriteResource & spriteResource);

private:
	static void unloadMeshResource (Mesh * mesh);
	static void unloadTextureResource (Texture2D * texture);
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

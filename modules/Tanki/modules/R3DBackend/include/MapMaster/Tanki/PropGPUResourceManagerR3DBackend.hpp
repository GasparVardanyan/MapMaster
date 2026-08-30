# pragma once

# include <memory>

# include <raylib.h>
# include <r3d/r3d_mesh.h>
# include <r3d/r3d_mesh_data.h>

# include "MapMaster/Tanki/PrimitiveFactory.hpp"
# include "MapMaster/Tanki/PrimitiveFactoryR3DBackend.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerR3DBackend.hpp"
# include "MapMaster/Tanki/PropLibrary.hpp"
# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/PropMetaData.hpp"

namespace MapMaster::Tanki {

// cppcheck-suppress-begin unusedStructMember
class PropGPUResourceManagerR3DBackend {
public:
	using CPUResourceManagerBackend = PropCPUResourceManagerR3DBackend;
	using CPUResourceManager = PropCPUResourceManager <CPUResourceManagerBackend>;
	using PrimitiveFactoryBackend = PrimitiveFactoryR3DBackend;
	using PrimitiveFactory = PrimitiveFactory <PrimitiveFactoryBackend>;

	struct MeshResource {
		std::shared_ptr <R3D_Mesh> mesh;
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
	static void unloadMeshResource (R3D_Mesh * mesh);
	static void unloadTextureResource (Texture2D * texture);
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

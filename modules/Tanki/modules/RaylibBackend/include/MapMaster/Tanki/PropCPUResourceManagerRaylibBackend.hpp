# pragma once

# include <cstdio>
# include <memory>
# include <string>
# include <type_traits>
# include <vector>

# include <raylib.h>

# include "MapMaster/CoreUtils/PropCPUResourceManagerUtils.hpp"
# include "MapMaster/Tanki/PrimitiveCPUFactory.hpp"
# include "MapMaster/Tanki/PrimitiveCPUFactoryRaylibBackend.hpp"
# include "MapMaster/Tanki/PropMetaData.hpp"

struct aiScene;
struct aiNode;

namespace MapMaster::Tanki::PropMetaData { struct Mesh; struct Texture; }



namespace MapMaster::Tanki {

// cppcheck-suppress-begin unusedStructMember
class PropCPUResourceManagerRaylibBackend {
public:
	static int AssimpImporterRemoveComponentFlags;
	static unsigned int AssimpPostProcessorSteps;

	struct PropMeshResource {
		using VertexType = float;
		using NormalType = float;
		using TexCoordType = float;
		using IndexType = unsigned short; // NOLINT(google-runtime-int)

		static_assert (std::is_floating_point_v <VertexType>);
		static_assert (std::is_floating_point_v <NormalType>);
		static_assert (std::is_floating_point_v <TexCoordType>);
		static_assert (std::is_integral_v <IndexType>);

		std::vector <VertexType> vertexBuffer;
		// std::vector <NormalType> normalBuffer;
		std::vector <TexCoordType> uvBuffer;
		std::vector <IndexType> indexBuffer;

		std::shared_ptr <PropMetaData::Mesh> meta;
	};

	struct PropTextureResource {
		std::shared_ptr <unsigned char> pixBuffer;
		std::shared_ptr <PropMetaData::Texture> meta;
	};

	struct PropSpriteResource {
		Vector2 size;
		Vector2 origin;
	};

	using PrimitiveCPUFactoryBackend = PrimitiveCPUFactoryRaylibBackend;
	using PrimitiveCPUFactory = PrimitiveCPUFactory <PrimitiveCPUFactoryBackend>;

	static PropMeshResource ParseMeshResource (const aiScene * scene);
	static PropTextureResource ParseTextureResource (CoreUtils::PropCPUResourceManagerUtils::Texture::FileUniquePtr diffuseFileHandle, CoreUtils::PropCPUResourceManagerUtils::Texture::FileUniquePtr alphaFileHandle);
	static PropSpriteResource GenerateSpriteResource (PropMetaData::Sprite meta);
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

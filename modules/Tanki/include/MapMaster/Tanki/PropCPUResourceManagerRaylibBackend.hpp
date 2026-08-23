# pragma once

# include <cstdio>
# include <memory>

# include <raylib.h>
# include <string>
# include <type_traits>
# include <vector>

struct aiScene;
struct aiNode;

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

		std::string textureFile;
		std::vector <VertexType> vertexBuffer;
		std::vector <NormalType> normalBuffer;
		std::vector <TexCoordType> uvBuffer;
		std::vector <IndexType> indexBuffer;
	};

	struct PropTextureResource {
		std::shared_ptr <unsigned char> pixBuffer;
		int width = -1;
		int height = -1;
		int channels = -1;

		// TODO: replace with big five
		PropTextureResource clone ();
	};

	static PropMeshResource ParseMeshResource (const aiScene * scene, const aiNode * visualNode);
	static PropTextureResource ParseTextureResource (std::FILE * diffuseFileHandle, std::FILE * alphaFileHandle);
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

# pragma once

# include <memory>

# include <raylib.h>
# include <string>
# include <type_traits>
# include <vector>

# include "MapMaster/Tanki/PropCPUResourceManagerRaylibAdapter.hpp"
# include "MapMaster/Tanki/PropLibrary.hpp"

namespace MapMaster::Tanki {

// TODO: make an interface trait
// cppcheck-suppress-begin unusedStructMember
class PropCPUResourceManagerRaylibAdapter {
public:
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
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

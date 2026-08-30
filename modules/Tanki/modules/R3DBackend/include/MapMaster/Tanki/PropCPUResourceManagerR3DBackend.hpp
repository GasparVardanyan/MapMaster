# pragma once

# include "MapMaster/Tanki/PropMetaData.hpp"
# include <cstdint>
# include <cstdio>
# include <memory>

# include <string>
# include <type_traits>
# include <vector>

# include <raylib.h>

# include <r3d/r3d_mesh_data.h>
# include <r3d/r3d_vertex.h>

struct aiScene;
struct aiNode;

namespace MapMaster::Tanki {

// cppcheck-suppress-begin unusedStructMember
class PropCPUResourceManagerR3DBackend {
public:
	static int AssimpImporterRemoveComponentFlags;
	static unsigned int AssimpPostProcessorSteps;

	struct PropMeshResource {
		std::shared_ptr <R3D_MeshData> mesh;
		static_assert (std::is_same_v <decltype (R3D_Vertex::position), Vector3>);
		static_assert (std::is_same_v <decltype (Vector3::x), float>);
		static_assert (std::is_same_v <decltype (Vector3::y), float>);
		static_assert (std::is_same_v <decltype (Vector3::z), float>);
		// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
		static_assert (std::is_same_v <decltype (R3D_Vertex::texcoord), uint16_t [2]>);
		// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
		static_assert (std::is_same_v <decltype (R3D_Vertex::normal), int8_t [4]>);
		static_assert (std::is_same_v <decltype (R3D_MeshData::indices), uint32_t *>);

		BoundingBox aabb;
		static_assert (std::is_same_v <decltype (BoundingBox::min), Vector3>);
		static_assert (std::is_same_v <decltype (BoundingBox::max), Vector3>);
		static_assert (std::is_same_v <decltype (Vector3::x), float>);
		static_assert (std::is_same_v <decltype (Vector3::y), float>);
		static_assert (std::is_same_v <decltype (Vector3::z), float>);

		PropMetaData::Mesh meta;
	};

	struct PropTextureResource {
		std::shared_ptr <unsigned char> pixBuffer;
		PropMetaData::Texture meta;
	};

	static PropMeshResource ParseMeshResource (const aiScene * scene, const aiNode * visualNode);
	static PropTextureResource ParseTextureResource (std::FILE * diffuseFileHandle, std::FILE * alphaFileHandle);
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

# pragma once // NOLINT(portability-avoid-pragma-once)

# include <string>
# include <type_traits>
# include <vector>

struct aiScene;
struct aiNode;

namespace MapMaster::Tanki {

namespace PropMetaData {

struct Mesh {
	using VertexType = float;
	static_assert (std::is_floating_point_v <VertexType>);

	struct Collider {
		using VertexType = float;

		static_assert (std::is_floating_point_v <VertexType>);

		struct BoxCollider {
			struct {
				VertexType x, y, z;
			} vMin, vMax;
		};
		struct RectCollider {
			struct {
				VertexType x, y, z;
			} v1, v2, v3, v4;
		};
		struct TriangleCollider {
			struct {
				VertexType x, y, z;
			} v1, v2, v3;
		};

		std::vector <BoxCollider> boxColliders;
		std::vector <RectCollider> rectColliders;
		std::vector <TriangleCollider> triangleColliders;
	};

	static Collider ParseCollider (const aiScene * scene, const aiNode * visualNode);

	struct {
		struct {
			VertexType x, y, z;
		} min, max;
	} aabb;

	std::string textureFile;
	Collider collider;
};

struct Texture {
	using SizeType = int;
	static_assert (std::is_integral_v <SizeType>);

	SizeType width, height, channels;
};

} // namespace PropMetaData

}  // namespace MapMaster::Tanki

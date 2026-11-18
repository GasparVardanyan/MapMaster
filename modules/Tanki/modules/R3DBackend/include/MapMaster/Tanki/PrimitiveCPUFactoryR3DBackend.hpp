# pragma once

# include <memory>

# include <r3d/r3d_mesh_data.h>

# include "MapMaster/Tanki/PropMetaData.hpp"

namespace MapMaster::Tanki {

class PrimitiveCPUFactoryR3DBackend {
public:
	struct ColliderResource {};
	struct SpriteResource {
		std::shared_ptr <R3D_MeshData> mesh;
	};

	static ColliderResource MakeColliderResource (const PropMetaData::Mesh::Collider::BoxCollider & /*meta*/) { return {}; }
	static ColliderResource MakeColliderResource (const PropMetaData::Mesh::Collider::RectCollider & /*meta*/) { return {}; }
	static ColliderResource MakeColliderResource (const PropMetaData::Mesh::Collider::TriangleCollider & /*meta*/) { return {}; }
	static SpriteResource MakeSpriteResource (const PropMetaData::Sprite & meta);
};

}  // namespace MapMaster::Tanki

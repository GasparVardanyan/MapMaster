# pragma once

# include <memory>

# include "MapMaster/Tanki/PropMetaData.hpp"

namespace MapMaster::Tanki {

class PrimitiveCPUFactoryRaylibBackend {
public:
	struct ColliderResource {};
	struct SpriteResource {};

	static ColliderResource MakeColliderResource (const PropMetaData::Mesh::Collider::BoxCollider & /*meta*/) { return {}; }
	static ColliderResource MakeColliderResource (const PropMetaData::Mesh::Collider::RectCollider & /*meta*/) { return {}; }
	static ColliderResource MakeColliderResource (const PropMetaData::Mesh::Collider::TriangleCollider & /*meta*/) { return {}; }
	static SpriteResource MakeSpriteResource (const PropMetaData::Sprite &  /*meta*/) { return {}; }
};

}  // namespace MapMaster::Tanki

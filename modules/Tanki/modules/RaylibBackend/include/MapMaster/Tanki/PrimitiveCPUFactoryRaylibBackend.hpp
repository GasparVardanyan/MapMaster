# pragma once

# include <memory>

# include "MapMaster/Tanki/PropMetaData.hpp"

namespace MapMaster::Tanki {

class PrimitiveCPUFactoryRaylibBackend {
public:
	struct ColliderResource {};
	struct SpriteResource {};

	std::shared_ptr <ColliderResource> getSharedColliderResource (const PropMetaData::Mesh::Collider::BoxCollider &  /*meta*/) { return {}; }
	std::shared_ptr <ColliderResource> getSharedColliderResource (const PropMetaData::Mesh::Collider::RectCollider &  /*meta*/) { return {}; }
	std::shared_ptr <ColliderResource> getSharedColliderResource (const PropMetaData::Mesh::Collider::TriangleCollider &  /*meta*/) { return {}; }
	std::shared_ptr <SpriteResource> getSharedSpriteResource (const PropMetaData::Sprite &  /*meta*/) { return {}; }
};

}  // namespace MapMaster::Tanki

# pragma once

# include <memory>

# include "MapMaster/Tanki/PropMetaData.hpp"

namespace MapMaster::Tanki {

class PrimitiveFactoryRaylibBackend {
public:
	struct SpriteMeshResource {};
	struct ColliderMeshResource {};

	std::shared_ptr <SpriteMeshResource> getSharedSpriteMeshResource (const PropMetaData::Sprite &  /*meta*/) { return {}; }
	std::shared_ptr <ColliderMeshResource> getSharedColliderMeshResource (const PropMetaData::Mesh::Collider::BoxCollider &  /*meta*/) { return {}; }
	std::shared_ptr <ColliderMeshResource> getSharedColliderMeshResource (const PropMetaData::Mesh::Collider::RectCollider &  /*meta*/) { return {}; }
	std::shared_ptr <ColliderMeshResource> getSharedColliderMeshResource (const PropMetaData::Mesh::Collider::TriangleCollider &  /*meta*/) { return {}; }
};

}  // namespace MapMaster::Tanki

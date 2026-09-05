# pragma once

# include <memory>
# include <type_traits>
# include <utility>

# include "MapMaster/Tanki/PropMetaData.hpp"

namespace MapMaster::Tanki {

template <class PrimitiveFactoryBackend, typename = void>
struct IsPrimitiveFactoryBackend : std::false_type {};

template <class PrimitiveFactoryBackend>
struct IsPrimitiveFactoryBackend <
	PrimitiveFactoryBackend,
	std::void_t <
		typename PrimitiveFactoryBackend::SpriteMeshResource,
		typename PrimitiveFactoryBackend::ColliderMeshResource,
		std::enable_if_t <std::is_same_v <
			std::shared_ptr <typename PrimitiveFactoryBackend::SpriteMeshResource>,
			decltype (std::declval <PrimitiveFactoryBackend> ().getSharedSpriteMeshResource (std::declval <const PropMetaData::Sprite &> ()))
		>>,
		std::enable_if_t <std::is_same_v <
			std::shared_ptr <typename PrimitiveFactoryBackend::ColliderMeshResource>,
			decltype (std::declval <PrimitiveFactoryBackend> ().getSharedColliderMeshResource (std::declval <const PropMetaData::Mesh::Collider::BoxCollider &> ()))
		>>,
		std::enable_if_t <std::is_same_v <
			std::shared_ptr <typename PrimitiveFactoryBackend::ColliderMeshResource>,
			decltype (std::declval <PrimitiveFactoryBackend> ().getSharedColliderMeshResource (std::declval <const PropMetaData::Mesh::Collider::RectCollider &> ()))
		>>,
		std::enable_if_t <std::is_same_v <
			std::shared_ptr <typename PrimitiveFactoryBackend::ColliderMeshResource>,
			decltype (std::declval <PrimitiveFactoryBackend> ().getSharedColliderMeshResource (std::declval <const PropMetaData::Mesh::Collider::TriangleCollider &> ()))
		>>
	>
> : std::true_type {};

template <class PrimitiveFactoryBackend>
class PrimitiveFactory : protected PrimitiveFactoryBackend{
public:
	using Backend = std::enable_if_t <
		IsPrimitiveFactoryBackend <PrimitiveFactoryBackend>::value,
		PrimitiveFactoryBackend
	>;

	using SpriteMeshResource = Backend::SpriteMeshResource;
	using ColliderMeshResource = Backend::ColliderMeshResource;

	using Backend::getSharedSpriteMeshResource;
	using Backend::getSharedColliderMeshResource;
};

}  // namespace MapMaster::Tanki

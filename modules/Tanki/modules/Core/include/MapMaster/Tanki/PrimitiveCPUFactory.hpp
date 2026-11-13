# pragma once

# include <memory>
# include <type_traits>
# include <utility>

# include "MapMaster/Tanki/PropMetaData.hpp"

namespace MapMaster::Tanki {

template <class PrimitiveCPUFactoryBackend, typename = void>
struct IsPrimitiveCPUFactoryBackend : std::false_type {};

template <class PrimitiveCPUFactoryBackend>
struct IsPrimitiveCPUFactoryBackend <
	PrimitiveCPUFactoryBackend,
	std::void_t <
		typename PrimitiveCPUFactoryBackend::ColliderResource,

		std::enable_if_t <std::is_same_v <
			std::shared_ptr <typename PrimitiveCPUFactoryBackend::ColliderResource>,
			decltype (std::declval <PrimitiveCPUFactoryBackend> ().getSharedColliderResource (std::declval <const PropMetaData::Mesh::Collider::BoxCollider &> ()))
		>>,
		std::enable_if_t <std::is_same_v <
			std::shared_ptr <typename PrimitiveCPUFactoryBackend::ColliderResource>,
			decltype (std::declval <PrimitiveCPUFactoryBackend> ().getSharedColliderResource (std::declval <const PropMetaData::Mesh::Collider::RectCollider &> ()))
		>>,
		std::enable_if_t <std::is_same_v <
			std::shared_ptr <typename PrimitiveCPUFactoryBackend::ColliderResource>,
			decltype (std::declval <PrimitiveCPUFactoryBackend> ().getSharedColliderResource (std::declval <const PropMetaData::Mesh::Collider::TriangleCollider &> ()))
		>>,

		typename PrimitiveCPUFactoryBackend::SpriteResource,
		std::enable_if_t <std::is_same_v <
			std::shared_ptr <typename PrimitiveCPUFactoryBackend::SpriteResource>,
			decltype (std::declval <PrimitiveCPUFactoryBackend> ().getSharedSpriteResource (std::declval <const PropMetaData::Sprite &> ()))
		>>
	>
> : std::true_type {};

template <class PrimitiveCPUFactoryBackend>
class PrimitiveCPUFactory : protected PrimitiveCPUFactoryBackend {
public:
	using Backend = std::enable_if_t <
		IsPrimitiveCPUFactoryBackend <PrimitiveCPUFactoryBackend>::value,
		PrimitiveCPUFactoryBackend
	>;

	using ColliderResource = Backend::ColliderResource;
	using SpriteResource = Backend::SpriteResource;

	using Backend::getSharedColliderResource;
	using Backend::getSharedSpriteResource;
};

}  // namespace MapMaster::Tanki

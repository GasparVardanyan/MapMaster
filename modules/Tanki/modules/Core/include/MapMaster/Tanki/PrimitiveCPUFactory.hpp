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
		typename PrimitiveCPUFactoryBackend::SpriteResource,

		// FIXME: decltype on overloaded function is ill-formed.
		//
		// std::enable_if_t <std::is_invocable_v <
		// 	decltype (PrimitiveCPUFactoryBackend::MakeColliderResource),
		// 	const PropMetaData::Mesh::Collider::BoxCollider &
		// >>,
		// std::enable_if_t <std::is_invocable_v <
		// 	decltype (PrimitiveCPUFactoryBackend::MakeColliderResource),
		// 	const PropMetaData::Mesh::Collider::RectCollider &
		// >>,
		// std::enable_if_t <std::is_invocable_v <
		// 	decltype (PrimitiveCPUFactoryBackend::MakeColliderResource),
		// 	const PropMetaData::Mesh::Collider::TriangleCollider &
		// >>,
		//
		// std::enable_if_t <std::is_invocable_v <
		// 	decltype (PrimitiveCPUFactoryBackend::MakeSpriteResource),
		// 	const PropMetaData::Sprite &
		// >>,
		//
		// std::enable_if_t <std::is_same_v <
		// 	typename PrimitiveCPUFactoryBackend::ColliderResource,
		// 	std::invoke_result_t <
		// 		decltype (PrimitiveCPUFactoryBackend::MakeColliderResource),
		// 		const PropMetaData::Mesh::Collider::BoxCollider &
		// 	>
		// >>,
		// std::enable_if_t <std::is_same_v <
		// 	typename PrimitiveCPUFactoryBackend::ColliderResource,
		// 	std::invoke_result_t <
		// 		decltype (PrimitiveCPUFactoryBackend::MakeColliderResource),
		// 		const PropMetaData::Mesh::Collider::RectCollider &
		// 	>
		// >>,
		// std::enable_if_t <std::is_same_v <
		// 	typename PrimitiveCPUFactoryBackend::ColliderResource,
		// 	std::invoke_result_t <
		// 		decltype (PrimitiveCPUFactoryBackend::MakeColliderResource),
		// 		const PropMetaData::Mesh::Collider::TriangleCollider &
		// 	>
		// >>,
		//
		// std::enable_if_t <std::is_same_v <
		// 	typename PrimitiveCPUFactoryBackend::SpriteResource,
		// 	std::invoke_result_t <
		// 		decltype (PrimitiveCPUFactoryBackend::MakeSpriteResource),
		// 		const PropMetaData::Sprite &
		// 	>
		// >>

		void
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
};

}  // namespace MapMaster::Tanki

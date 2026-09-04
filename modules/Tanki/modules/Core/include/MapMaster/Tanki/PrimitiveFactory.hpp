# pragma once

# include <memory>
# include <type_traits>

# include "MapMaster/Tanki/PropMetaData.hpp"

namespace MapMaster::Tanki {

template <class PrimitiveFactoryBackend, typename = void>
struct IsPrimitiveFactoryBackend : std::false_type {};

template <class PrimitiveFactoryBackend>
struct IsPrimitiveFactoryBackend <
	PrimitiveFactoryBackend,
	std::void_t <
		typename PrimitiveFactoryBackend::SpriteMeshResource,
		typename PrimitiveFactoryBackend::TriangleColliderMeshResource,
		typename PrimitiveFactoryBackend::RectColliderMeshResource,
		typename PrimitiveFactoryBackend::BoxColliderMeshResource
	>
> : std::true_type {};

template <class PrimitiveFactoryBackend>
class PrimitiveFactory {
public:
	using Backend = std::enable_if_t <
		IsPrimitiveFactoryBackend <PrimitiveFactoryBackend>::value,
		PrimitiveFactoryBackend
	>;

	using SpriteMeshResource = Backend::SpriteMeshResource;
	using TriangleColliderMeshResource = Backend::TriangleColliderMeshResource;
	using RectColliderMeshResource = Backend::RectColliderMeshResource;
	using BoxColliderMeshResource = Backend::BoxColliderMeshResource;

	std::shared_ptr <SpriteMeshResource> GetSharedSpriteMeshResource (const PropMetaData::Sprite & meta);
};

}  // namespace MapMaster::Tanki

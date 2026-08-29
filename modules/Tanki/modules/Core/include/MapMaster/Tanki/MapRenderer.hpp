# pragma once

# include "MapMaster/Tanki/Map.hpp"

# include <memory>
# include <type_traits>
# include <utility>

namespace MapMaster::Tanki {

template <class MapRendererBackend, typename = void>
struct IsMapRendererBackend : std::false_type {};

template <class MapRendererBackend>
struct IsMapRendererBackend <
	MapRendererBackend,
	std::void_t <
		typename MapRendererBackend::CPUResourceManager,
		typename MapRendererBackend::GPUResourceManager,
		typename MapRendererBackend::Camera,
		typename MapRendererBackend::Color,
		typename MapRendererBackend::SceneTriangleCollider,
		typename MapRendererBackend::SceneRectCollider,
		typename MapRendererBackend::SceneBoxCollider,
		typename MapRendererBackend::SceneMesh,
		typename MapRendererBackend::SceneSprite,

		std::enable_if_t <std::is_same_v <
			std::shared_ptr <typename MapRendererBackend::GPUResourceManager>,
			decltype (std::declval <const MapRendererBackend> ().resourceManager ())
		>>,

		decltype (std::declval <MapRendererBackend> ().setResourceManager (
			std::declval <std::shared_ptr <typename MapRendererBackend::GPUResourceManager>> ())
		),

		std::enable_if_t <std::is_same_v <
			std::shared_ptr <Map>,
			decltype (std::declval <const MapRendererBackend> ().map ())
		>>,

		decltype (std::declval <MapRendererBackend> ().setMap (
			std::declval <std::shared_ptr <Map>> ())
		),

		decltype (std::declval <MapRendererBackend> ().loadScene (
			std::declval <float> ())
		),

		decltype (std::declval <MapRendererBackend> ().render (
			std::declval <typename MapRendererBackend::Camera &> ())
		),

		decltype (std::declval <MapRendererBackend> ().renderCollisionGeometry (
			std::declval <bool> ())
		),

		decltype (std::declval <MapRendererBackend> ().setCollisionGeometryFaceColor (
			std::declval <typename MapRendererBackend::Color> ())
		),
		std::enable_if_t <std::is_same_v <
			typename MapRendererBackend::Color,
			decltype (std::declval <const MapRendererBackend> ().collisionGeometryFaceColor ())
		>>,

		decltype (std::declval <MapRendererBackend> ().setCollisionGeometryEdgeColor (
			std::declval <typename MapRendererBackend::Color> ())
		),
		std::enable_if_t <std::is_same_v <
			typename MapRendererBackend::Color,
			decltype (std::declval <const MapRendererBackend> ().collisionGeometryEdgeColor ())
		>>
	>
> : std::true_type {};

template <class MapRendererBackend>
class MapRenderer : protected MapRendererBackend {
public:
	using Backend = std::enable_if_t <
		IsMapRendererBackend <MapRendererBackend>::value,
		MapRendererBackend
	>;

	using CPUResourceManager = Backend::CPUResourceManager;
	using GPUResourceManager = Backend::GPUResourceManager;

	using SceneTriangleCollider = Backend::SceneTriangleCollider;
	using SceneRectCollider = Backend::SceneRectCollider;
	using SceneBoxCollider = Backend::SceneBoxCollider;

	using SceneMesh = Backend::SceneMesh;
	using SceneSprite = Backend::SceneSprite;

	using Backend::setResourceManager;
	using Backend::resourceManager;
	using Backend::setMap;
	using Backend::map;

	using Backend::loadScene;
	using Backend::render;
	using Backend::renderCollisionGeometry;

	using Backend::setCollisionGeometryFaceColor;
	using Backend::setCollisionGeometryEdgeColor;
	using Backend::collisionGeometryFaceColor;
	using Backend::collisionGeometryEdgeColor;
};

}  // namespace MapMaster::Tanki

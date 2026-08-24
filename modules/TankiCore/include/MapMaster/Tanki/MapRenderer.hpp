# pragma once

namespace MapMaster::Tanki {

template <class MapRendererBackend>
class MapRenderer : protected MapRendererBackend {
public:
	using RendeingBackend = MapRendererBackend;

	using CPUResourceManager = RendeingBackend::CPUResourceManager;
	using GPUResourceManager = RendeingBackend::GPUResourceManager;

	using SceneTriangleCollider = RendeingBackend::SceneTriangleCollider;
	using SceneRectCollider = RendeingBackend::SceneRectCollider;
	using SceneBoxCollider = RendeingBackend::SceneBoxCollider;

	using SceneMesh = RendeingBackend::SceneMesh;
	using SceneSprite = RendeingBackend::SceneSprite;

	using RendeingBackend::setResourceManager;
	using RendeingBackend::resourceManager;
	using RendeingBackend::setMap;
	using RendeingBackend::map;

	using RendeingBackend::loadScene;
	using RendeingBackend::render;
	using RendeingBackend::renderCollisionGeometry;

	using RendeingBackend::setCollisionGeometryFaceColor;
	using RendeingBackend::setCollisionGeometryEdgeColor;
	using RendeingBackend::collisionGeometryFaceColor;
	using RendeingBackend::collisionGeometryEdgeColor;
};

}  // namespace MapMaster::Tanki

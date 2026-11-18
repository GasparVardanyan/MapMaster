# include <memory>

# include <r3d/r3d_mesh_data.h>

# include "MapMaster/Tanki/PrimitiveCPUFactoryR3DBackend.hpp"
# include "MapMaster/Tanki/PropMetaData.hpp"

using namespace MapMaster::Tanki;

PrimitiveCPUFactoryR3DBackend::SpriteResource PrimitiveCPUFactoryR3DBackend::MakeSpriteResource(const PropMetaData::Sprite & meta) {
	R3D_MeshData spriteMesh = R3D_GenMeshDataQuad (
		static_cast <float> (meta.width * meta.scale),
		static_cast <float> (meta.height * meta.scale),
		1,
		1,
		{ .x = 0, .y = 0, .z = 1 }
	);

	float originDeltaW = static_cast <float> ((meta.originX - 0.5) * meta.width * meta.scale);
	float originDeltaH = static_cast <float> ((meta.originY - 0.5) * meta.height * meta.scale);

	for (int i = 0; i < spriteMesh.vertexCount; i++) {
		spriteMesh.vertices [i].position.x += originDeltaW;
		spriteMesh.vertices [i].position.y += originDeltaH;
	}
	return {
		.mesh = std::shared_ptr <R3D_MeshData> (new R3D_MeshData (spriteMesh), [] (R3D_MeshData * meshData) -> void {
			R3D_UnloadMeshData (* meshData);
			delete meshData;
		}),
	};
}

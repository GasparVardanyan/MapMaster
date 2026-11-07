# include "MapMaster/Tanki/Utils/R3DMeshUtils.hpp"

# include "r3d/r3d_mesh_data.h"

namespace MapMaster::Tanki::Utils {
namespace R3DMeshUtils {

R3D_MeshData GenerateSpriteMesh (double width, double height, double originX, double originY, double scale) {
	return R3D_GenMeshDataQuad (
		static_cast <float> (width * scale),
		static_cast <float> (height * scale),
		1,
		1,
		{ .x=0, .y=0, .z=1}
	);
}

} // namespace MeshUtils
} // namespace MapMaster::Tanki::Utils::R3DBackend

# include <chrono>
# include <iostream>
# include <memory>
# include <string>

# include <raylib.h>
# include <r3d/r3d.h>

# include "MapMaster/Utils/Tanki.hpp"

namespace MapMaster::Tanki {
class MapRenderer;
} // namespace MapMaster::Tanki



static constexpr float scale = 0.01F;

int main (int argc, char ** argv) {
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	MapMaster::Utils::Tanki::Window::OpenRaylibWindow (1680, 1050, "MapMaster", LOG_NONE);

	std::string propLibsRoot, mapFile;

	if (argc >= 3) {
		propLibsRoot = argv [1];// NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		mapFile = argv [2];
	}
	else {
		return 0;
	}

	std::cout << "LOADING: " << propLibsRoot << " : " << mapFile << '\n';

	std::chrono::time_point start = std::chrono::steady_clock::now ();

	std::shared_ptr <MapMaster::Tanki::MapRenderer> rmap = MapMaster::Utils::Tanki::LoadMapRenderer (propLibsRoot, mapFile, scale, false);

	std::chrono::time_point end = std::chrono::steady_clock::now ();

	std::cout << "Elapsed: "
		<< std::chrono::duration <double> (end - start).count ()
		<< " s\n";

	MapMaster::Utils::Tanki::Window::DrawMapRendererInCurrentWindow (rmap, scale, propLibsRoot, mapFile);

	rmap.reset ();


	// R3D_Init( 1680, 1050 );
	//
	// // Create scene objects
	// R3D_Mesh mesh = R3D_GenMeshSphere(1.0f, 16, 32);
	// R3D_Material material = R3D_GetDefaultMaterial();
	//
	// // Setup lighting
	// R3D_Light light = R3D_CreateDirLight((Vector3) {-1, -1, -1}, WHITE, 1.0f);
	//
	// // Camera setup
	// Camera3D camera = {
	// 	.position = {3, 3, 3},
	// 	.target = {0, 0, 0},
	// 	.up = {0, 1, 0},
	// 	.fovy = 60.0f,
	// 	.projection = CAMERA_PERSPECTIVE
	// };
	//
	// // Main loop
	// while (!WindowShouldClose()) {
	// 	UpdateCamera(&camera, CAMERA_ORBITAL);
	// 	BeginDrawing();
	// 	R3D_Begin(camera);
	// 	R3D_PushLight(light);
	// 	R3D_DrawMesh(mesh, material, Vector3Zero(), 1.0f);
	// 	R3D_End();
	// 	EndDrawing();
	// }
	//
	// R3D_UnloadMesh(mesh);
	// R3D_Close();

	MapMaster::Utils::Tanki::Window::CloseRaylibWindow ();
}

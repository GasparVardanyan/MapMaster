# include <chrono>
# include <iostream>
# include <memory>

# include <raylib.h>
# include <rlgl.h>

# include "RaylibMap.hpp"



static constexpr float scale = 0.01F;

// NOLINTBEGIN(*)
int main (void)
{
	const int screenWidth = 800;
	const int screenHeight = 450;

	// SetTraceLogLevel (LOG_NONE);
	SetConfigFlags (FLAG_MSAA_4X_HINT);
	InitWindow (screenWidth, screenHeight, "MapMaster");
	auto start = std::chrono::steady_clock::now ();

	std::shared_ptr <RaylibMap> rmap = std::make_shared <RaylibMap> ();
	rmap->map ()->loadFile (DATA_DIR "maps/M/map_silence_remake_cy95v_summer/map.xml");
	rmap->resourceManager ()->loadMapLibraries (* rmap->map (), DATA_DIR "propslibs");
	rmap->resourceManager ()->loadMapResources (* rmap->map ());
	rmap->loadScene (scale);

	auto end = std::chrono::steady_clock::now ();

	std::cout << "Elapsed: "
		<< std::chrono::duration <double> (end - start).count ()
		<< " s\n";


	Camera camera = { 0 };
	camera.position = { 50.0f, 50.0f, 50.0f };
	camera.target = { 0.0f, 12.0f, 0.0f };
	camera.up = { 0.0f, 0.0f, 1.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	SetTargetFPS (60);

	while (!WindowShouldClose ())
	{
		UpdateCamera (& camera, CAMERA_THIRD_PERSON);

		BeginDrawing ();

		ClearBackground (RAYWHITE);

		BeginMode3D (camera);

		rmap->render (camera);

		rlPushMatrix ();
		rlRotatef (90.0f, 1.0f, 0.0f, 0.0f);
		DrawGrid (50, 5.0f);
		rlPopMatrix ();

		EndMode3D ();

		DrawFPS (10, 10);

		EndDrawing ();
	}

	rmap.reset ();

	CloseWindow ();

	return 0;
}
// NOLINTEND(*)


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

	// this loads the map xml data
	rmap->map ()->loadFile (DATA_DIR "maps/M/map_silence_remake_cy95v_summer/map.xml");
	// rmap->map ()->loadFile (DATA_DIR "finalboss.xml");
	// rmap->map ()->loadFile (DATA_DIR "maps/Summer/Sandbox_MM.xml");
	// rmap->map ()->loadFile (DATA_DIR "maps/M/map_sandbox_2.0_summer/map.xml");
	// rmap->map ()->loadFile (DATA_DIR "maps/M/map_tutorial_summer/map.xml");
	// rmap->map ()->loadFile (DATA_DIR "map.xml");

	// this loads the xml data of the necessary proplibs from the given directory
	// all proplibs must be directly in this directory and their directories must
	// be named like their names specified in library.xml files
	//
	// otherwise manually load the directories with the loadLibrary method
	// rmap->resourceManager ()->loadMapLibraries (* rmap->map (), DATA_DIR "PLVK");
	rmap->resourceManager ()->loadMapLibraries (* rmap->map (), DATA_DIR "propslibs");

	// this loads all the necessary mesh and texture resources to the gpu and
	// generates sprite info for rendering
	rmap->resourceManager ()->loadMapResources (* rmap->map ());

	// this creates the map scene
	rmap->loadScene (scale);

	auto end = std::chrono::steady_clock::now ();

	std::cout << "Elapsed: "
		<< std::chrono::duration <double> (end - start).count ()
		<< " s\n";


	Camera camera = {
		.position = {
			.x = 50.0f,
			.y = 50.0f,
			.z = 50.0f,
		},
		.target = {
			.x = 0.0f, .y = 12.0f, .z = 0.0f
		},
		.up = {
			.x = 0.0f,
			.y = 0.0f,
			.z = 1.0f,
		},
		.fovy = 45.0f,
		.projection = CAMERA_PERSPECTIVE,
	};

	SetTargetFPS (60);

	while (false == WindowShouldClose ()) {
		UpdateCamera (& camera, CAMERA_THIRD_PERSON);

		BeginDrawing ();

		ClearBackground (RAYWHITE);

		BeginMode3D (camera);

		rmap->render (camera);

		rlPushMatrix ();
		rlRotatef (90.0f, 1.0f, 0.0f, 0.0f);
		DrawGrid (50, 500.0f * scale);
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

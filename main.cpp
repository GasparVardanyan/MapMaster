# include <chrono>
# include <iostream>
# include <memory>
# include <string>

# include <raylib.h>
# include <rlgl.h>

# include "MapMaster/Scene3D/CameraController.hpp"
# include "MapMaster/Tanki/RaylibMap.hpp"
# include "MapMaster/Tanki/RaylibPropResourceManager.hpp"



using namespace MapMaster::Tanki;



static constexpr float scale = 0.01F;

int main (int argc, char ** argv)
{
	// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

	std::string propLibsRoot, mapFile;

	if (argc >= 3) {
		propLibsRoot = argv [1];
		mapFile = argv [2];
	}
	else {
		return 0;
	}
	const int screenWidth = 800;
	const int screenHeight = 450;

	SetTraceLogLevel (LOG_NONE);
	SetConfigFlags (FLAG_MSAA_4X_HINT);
	InitWindow (screenWidth, screenHeight, "MapMaster");

	std::chrono::time_point start = std::chrono::steady_clock::now ();

	std::shared_ptr <RaylibMap> rmap = std::make_shared <RaylibMap> ();
	rmap->setResourceManager (std::make_shared <RaylibPropResourceManager> (true));

	// this loads the map xml data
	rmap->map ()->loadFile (mapFile);

	// this loads the xml data of the necessary proplibs from the given directory
	// all proplibs must be directly in this directory and their directories must
	// be named like their names specified in library.xml files
	//
	// otherwise manually load the directories with the loadLibrary method
	// rmap->resourceManager ()->loadMapLibraries (* rmap->map (), DATA_DIR "PLVK");
	rmap->resourceManager ()->loadMapLibraries (* rmap->map (), propLibsRoot);

	// this loads all the necessary mesh and texture resources to the gpu and
	// generates sprite info for rendering
	rmap->resourceManager ()->loadMapResources (* rmap->map ());

	// this creates the map scene
	rmap->loadScene (scale);

	std::chrono::time_point end = std::chrono::steady_clock::now ();

	std::cout << "Elapsed: "
		<< std::chrono::duration <double> (end - start).count ()
		<< " s\n";


	Camera camera = {
		.position = {
			.x = 50.0F,
			.y = 50.0F,
			.z = 50.0F,
		},
		.target = {
			.x = 0.0F, .y = 12.0F, .z = 0.0F
		},
		.up = {
			.x = 0.0F,
			.y = 0.0F,
			.z = 1.0F,
		},
		.fovy = 45.0F,
		.projection = CAMERA_PERSPECTIVE,
	};

	SetTargetFPS (60);

	bool drawCollisionGeometry = false;

	CameraController cameraController;
	cameraController.setCamera (& camera);
	cameraController.setMoveSpeed (cameraController.moveSpeed () * scale);

	while (false == WindowShouldClose ()) {
		// UpdateCamera (& camera, CAMERA_THIRD_PERSON);
		cameraController.updateCamera ();

		if (true == IsKeyPressed (KEY_SPACE)) {
			drawCollisionGeometry = !drawCollisionGeometry;
		}

		BeginDrawing ();

		ClearBackground ({.r = 0x22, .g = 0x44, .b = 0x66, .a = 0xFF});

		BeginMode3D (camera);

		if (true == drawCollisionGeometry) {
			rmap->renderCollisionGeometry ();
		}
		else {
			rmap->render (camera);

			rlPushMatrix ();
			rlRotatef (90.0F, 1.0F, 0.0F, 0.0F);
			DrawGrid (50, 500.0F * scale);
			rlPopMatrix ();
		}

		EndMode3D ();

		DrawFPS (10, 10);

		EndDrawing ();
	}

	rmap.reset ();

	CloseWindow ();

	return 0;

	// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

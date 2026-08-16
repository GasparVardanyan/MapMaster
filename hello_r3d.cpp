# include <assimp/Importer.hpp>
# include <assimp/postprocess.h>
# include <assimp/scene.h>
# include <cstdio>
# include <string>

# include <raylib.h>
# include <rlgl.h>
# include <raymath.h>
# include <r3d/r3d.h>
# include <r3d/r3d_core.h>
# include <r3d/r3d_draw.h>
# include <r3d/r3d_lighting.h>
# include <r3d/r3d_material.h>
# include <r3d/r3d_mesh.h>

# include "MapMaster/Scene3D/CameraController.hpp"
# include "MapMaster/Tanki/Map.hpp"
# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerRaylibBackend.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerR3DBackend.hpp"
# include "MapMaster/Tanki/PropGPUResourceManagerRaylibBackend.hpp"
# include "MapMaster/Utils/Tanki.hpp"
# include "r3d/r3d_mesh_data.h"
# include "r3d/r3d_model.h"
# include "r3d/r3d_texture.h"

using RenderingCPUBackend = MapMaster::Tanki::PropCPUResourceManagerRaylibBackend;

namespace MapMaster::Tanki {
class MapRenderer;
}

template <typename PropCPUResourceManagerBackend>
static MapMaster::Tanki::PropCPUResourceManager <PropCPUResourceManagerBackend>::PropMeshResource cpuMesh (const std::string & meshPath) {
	Assimp::Importer importer;
	importer.SetPropertyInteger (
		AI_CONFIG_PP_RVC_FLAGS,
		// aiComponent_NORMALS |
		aiComponent_TANGENTS_AND_BITANGENTS |
		aiComponent_COLORS |
		// aiComponent_TEXCOORDS |
		aiComponent_BONEWEIGHTS |
		aiComponent_ANIMATIONS |
		aiComponent_TEXTURES |
		aiComponent_LIGHTS |
		aiComponent_CAMERAS
		// aiComponent_MESHES |
		// aiComponent_MATERIALS
	);

	// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic,readability-math-missing-parentheses)

	const aiScene * scene = importer.ReadFile (meshPath, aiProcess_Triangulate | aiProcess_RemoveComponent | aiProcess_FlipUVs);

	const aiNode * visualNode = scene->mRootNode->mChildren [0];

	return PropCPUResourceManagerBackend::ParseMeshResource (scene, visualNode);
}

template <typename PropCPUResourceManagerBackend>
static MapMaster::Tanki::PropCPUResourceManager<PropCPUResourceManagerBackend>::PropTextureResource cpuTexture (const std::string & diffusePath) {
	std::FILE * diffuseFileHandle = std::fopen (diffusePath.c_str (), "rb");
	typename PropCPUResourceManagerBackend::PropTextureResource resource = PropCPUResourceManagerBackend::ParseTextureResource (diffuseFileHandle, nullptr);
	std::fclose (diffuseFileHandle);
	return resource;
}



static constexpr float scale = 0.01F;

int main (int argc, char ** argv) {
	// NOLINTBEGIN(*)
	MapMaster::Utils::Tanki::Window::OpenRaylibWindow (1680, 1050, "Hello R3D", LOG_NONE);

# if 0

	Camera camera = { 0 };
	camera.position = (Vector3){ 50.0f, 50.0f, 50.0f };
	camera.target = (Vector3){ 0.0f, 12.0f, 0.0f };
	camera.up = (Vector3){ 0.0f, 0.0f, 1.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	Model model = LoadModel("/src/data/MapMaster/castle.obj");
	Texture2D texture = LoadTexture("/src/data/MapMaster/castle_diffuse.png");
	model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

	struct {
		RenderingCPUBackend::PropMeshResource mesh;
		RenderingCPUBackend::PropTextureResource texture;
		Material material;
		Matrix transform;
	} cpuData {
		.mesh = cpuMesh <RenderingCPUBackend> ("/src/data/MapMaster/fahwerk1.3ds"),
		.texture = cpuTexture <RenderingCPUBackend> ("/src/data/MapMaster/fahwerk1.jpg"),
		.material = LoadMaterialDefault (),
		.transform = MatrixScale (scale, scale, scale),
	};

	struct {
		MapMaster::Tanki::PropGPUResourceManagerRaylibBackend::MeshResource mesh;
		MapMaster::Tanki::PropGPUResourceManagerRaylibBackend::TextureResource texture;
	} gpuData {
		.mesh = MapMaster::Tanki::PropGPUResourceManagerRaylibBackend::CreateMeshResource <true> (cpuData.mesh),
		.texture = MapMaster::Tanki::PropGPUResourceManagerRaylibBackend::CreateTextureResource <true> (cpuData.texture),
	};

	cpuData.material.maps [MATERIAL_MAP_DIFFUSE].texture = * gpuData.texture.texture;


	Vector3 position = { 0.0f, 0.0f, 0.0f };

	MapMaster::Scene3D::CameraController camCtrl;
	camCtrl.setCamera (& camera);
	camCtrl.setMoveSpeed (camCtrl.moveSpeed () * scale);

	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		camCtrl.updateCamera ();

		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(camera);

		// DrawModel(model, position, 1.0f, WHITE);
		DrawMesh(*gpuData.mesh.mesh, cpuData.material, cpuData.transform);

		rlPushMatrix ();
		rlRotatef (90.0F, 1.0F, 0.0F, 0.0F);
		DrawGrid (50, 500.0F * scale);
		rlPopMatrix ();

		EndMode3D();

		DrawFPS(10, 10);

		EndDrawing();
	}

	UnloadTexture(texture);
	UnloadModel(model);

	gpuData = {};
	cpuData = {};

# else

	R3D_Init( 1680, 1050 );


	R3D_Mesh mesh = R3D_GenMeshSphere(1.0f / scale, 16, 32);
	R3D_Material material = R3D_GetDefaultMaterial();

	{
		MapMaster::Tanki::PropCPUResourceManagerR3DBackend::PropMeshResource meshRes = cpuMesh <MapMaster::Tanki::PropCPUResourceManagerR3DBackend> ("/src/data/MapMaster/fahwerk1.3ds");
		R3D_Mesh m = R3D_LoadMesh (R3D_PrimitiveType::R3D_PRIMITIVE_TRIANGLES, * meshRes.mesh, & meshRes.aabb);
		// R3D_UnloadMesh (m);
		R3D_UnloadMesh (mesh);
		mesh = m;
	}

	R3D_Model model = R3D_LoadModel("/src/data/MapMaster/fahwerk1.3ds");
	Texture2D texture = R3D_LoadTexture ("/src/data/MapMaster/fahwerk1.jpg", false);
	model.materials [0].albedo.texture = texture;


	R3D_Light light = R3D_CreateDirLight((Vector3) {-1, -1, -1}, WHITE, 1.0f);


	Camera camera = { 0 };
	camera.position = (Vector3){ 50.0f, 50.0f, 50.0f };
	camera.target = (Vector3){ 0.0f, 12.0f, 0.0f };
	camera.up = (Vector3){ 0.0f, 0.0f, 1.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;


	while (!WindowShouldClose()) {
		UpdateCamera(&camera, CAMERA_ORBITAL);
		BeginDrawing();
		R3D_Begin(camera);
		R3D_PushLight(light);
		// R3D_DrawModel(model, Vector3Zero(), scale);
		R3D_DrawMesh(mesh, material, Vector3Zero(), scale);
		R3D_End();
		EndDrawing();
	}

	R3D_UnloadModel (model, true);
	R3D_UnloadMesh(mesh);
	R3D_Close();

# endif

	MapMaster::Utils::Tanki::Window::CloseRaylibWindow ();

	// NOLINTEND(*)
}

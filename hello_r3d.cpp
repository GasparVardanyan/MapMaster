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
# include "MapMaster/Tanki/MapRendererR3DBackend.hpp"
# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerRaylibBackend.hpp"
# include "MapMaster/Tanki/PropCPUResourceManagerR3DBackend.hpp"
# include "MapMaster/Tanki/PropGPUResourceManagerRaylibBackend.hpp"
# include "MapMaster/Utils/Tanki.hpp"
# include "r3d/r3d_mesh_data.h"
# include "r3d/r3d_model.h"
# include "r3d/r3d_texture.h"

using RenderingCPUBackend = MapMaster::Tanki::PropCPUResourceManagerRaylibBackend;

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

using MapRendererBackend = MapMaster::Tanki::MapRendererR3DBackend;



static constexpr float scale = 0.01;

int main (int argc, char ** argv) {
	// NOLINTBEGIN(*)
	MapMaster::Utils::Tanki::Window::OpenMapWindow <MapRendererBackend> (1680, 1050, "MapMaster", LOG_NONE);

	R3D_Init( 1680, 1050 );

	R3D_Material material = R3D_GetDefaultMaterial();

	MapMaster::Tanki::PropCPUResourceManagerR3DBackend::PropMeshResource meshRes;
	for (int i = 0; i < 10000; i++) {
		meshRes = cpuMesh <MapMaster::Tanki::PropCPUResourceManagerR3DBackend> ("/src/data/MapMaster/propslibs/Broken Roof/h_roofc.3ds");
	}

	R3D_Mesh mesh;

	mesh = R3D_LoadMesh (R3D_PrimitiveType::R3D_PRIMITIVE_TRIANGLES, * meshRes.mesh, & meshRes.aabb);

	MapMaster::Tanki::PropCPUResourceManagerR3DBackend::PropTextureResource textureRes = cpuTexture <MapMaster::Tanki::PropCPUResourceManagerR3DBackend> ("/src/data/MapMaster/fahwerk1.png");

	int pixelFormat = PIXELFORMAT_UNCOMPRESSED_R8G8B8;

	if (4 == textureRes.channels) {
		pixelFormat = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
	}
	Image i = {
		.data = static_cast <void *> (textureRes.pixBuffer.get ()),
		.width = textureRes.width,
		.height = textureRes.height,
		.mipmaps = 1,
		.format = pixelFormat
	};

	Texture2D texture = R3D_LoadTextureFromImage (i, true);
	material.albedo.texture = texture;

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

	material.albedo.texture = {};

	R3D_UnloadMesh (mesh);
	// R3D_UnloadMaterial (material);
	// R3D_UnloadTexture (texture);

	MapMaster::Utils::Tanki::Window::CloseMapWindow <MapRendererBackend> ();

	// NOLINTEND(*)
}

# include "MapMaster/Tanki/PropGPUResourceManagerR3DBackend.hpp"

# include <memory>

# include <raylib.h>
# include <r3d/r3d_mesh.h>
# include <r3d/r3d_mesh_data.h>
# include <r3d/r3d_texture.h>

using namespace MapMaster::Tanki;

template PropGPUResourceManagerR3DBackend::MeshResource
PropGPUResourceManagerR3DBackend::CreateMeshResource <true> (
	CPUResourceManager::PropMeshResource &
);

template PropGPUResourceManagerR3DBackend::MeshResource
PropGPUResourceManagerR3DBackend::CreateMeshResource <false> (
	CPUResourceManager::PropMeshResource &
);

template PropGPUResourceManagerR3DBackend::TextureResource
PropGPUResourceManagerR3DBackend::CreateTextureResource <true> (
	const CPUResourceManager::PropTextureResource &
);

template PropGPUResourceManagerR3DBackend::TextureResource
PropGPUResourceManagerR3DBackend::CreateTextureResource <false> (
	const CPUResourceManager::PropTextureResource &
);

template <bool Upload>
PropGPUResourceManagerR3DBackend::MeshResource PropGPUResourceManagerR3DBackend::CreateMeshResource (CPUResourceManager::PropMeshResource & meshResource) {
	MeshResource m = {};

	if constexpr (true == Upload) {
		m.mesh = std::shared_ptr <R3D_Mesh> (
			new R3D_Mesh (R3D_LoadMesh (R3D_PrimitiveType::R3D_PRIMITIVE_TRIANGLES, * meshResource.mesh, & m.aabb)),
			unloadMeshResource
		);
	}
	else {
		m.meshData = meshResource.mesh;
	}

	return m;
}

template <bool Upload>
// cppcheck-suppress functionStatic
PropGPUResourceManagerR3DBackend::TextureResource PropGPUResourceManagerR3DBackend::CreateTextureResource (const CPUResourceManager::PropTextureResource & textureResource) {
	TextureResource t = {};

	int pixelFormat = PIXELFORMAT_UNCOMPRESSED_R8G8B8;

	if (4 == textureResource.channels) {
		pixelFormat = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
	}

	t.image = {
		.data = static_cast <void *> (textureResource.pixBuffer.get ()),
		.width = textureResource.width,
		.height = textureResource.height,
		.mipmaps = 1,
		.format = pixelFormat
	};

	if constexpr (true == Upload) {
		t.texture = std::shared_ptr <Texture2D> (new Texture2D (R3D_LoadTextureFromImage (t.image, true)), unloadTextureResource);

		GenTextureMipmaps (t.texture.get ());
		SetTextureFilter (* t.texture, TEXTURE_FILTER_TRILINEAR);
	}

	return t;
}

void PropGPUResourceManagerR3DBackend::UploadMeshResource (MeshResource & meshResource) {
	meshResource.mesh = std::shared_ptr <R3D_Mesh> (
		new R3D_Mesh (R3D_LoadMesh (R3D_PrimitiveType::R3D_PRIMITIVE_TRIANGLES, * meshResource.meshData, & meshResource.aabb)),
		unloadMeshResource
	);
}

void PropGPUResourceManagerR3DBackend::UploadTextureResource (TextureResource & textureResource) {
	textureResource.texture = std::shared_ptr <Texture2D> (new Texture2D (R3D_LoadTextureFromImage (textureResource.image, true)), unloadTextureResource);

	GenTextureMipmaps (textureResource.texture.get ());
	SetTextureFilter (* textureResource.texture, TEXTURE_FILTER_TRILINEAR);
}

void PropGPUResourceManagerR3DBackend::unloadMeshResource (R3D_Mesh * mesh) {
	R3D_UnloadMesh (* mesh);
	// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
	delete mesh;
}

void PropGPUResourceManagerR3DBackend::unloadTextureResource (Texture2D * texture) {
	R3D_UnloadTexture (* texture);
	// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
	delete texture;
}

PropGPUResourceManagerR3DBackend::SpriteInfo PropGPUResourceManagerR3DBackend::CreateSpriteInfo (const PropLibrary::PropSprite & sprite, const CPUResourceManager::PropTextureResource & textureResource) {
	const Vector2 size = {
		.x = static_cast <float> (textureResource.width * sprite.scale),
		.y = static_cast <float> (textureResource.height * sprite.scale),
	};

	return {
		.origin = {
			.x = static_cast <float> (sprite.originX * size.x),
			.y = static_cast <float> ((1 - sprite.originY) * size.y),
		},
		.size = size,
	};
}

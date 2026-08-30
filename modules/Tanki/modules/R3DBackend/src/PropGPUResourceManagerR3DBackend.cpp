# include "MapMaster/Tanki/PropGPUResourceManagerR3DBackend.hpp"

# include <memory>

# include <raylib.h>
# include <r3d/r3d_mesh.h>
# include <r3d/r3d_mesh_data.h>
# include <r3d/r3d_texture.h>

# include "MapMaster/Tanki/PropGPUResourceManager.hpp"
# include "MapMaster/Tanki/PropGPUResourceManager.inl" // IWYU pragma: keep
# include "MapMaster/Tanki/PropLibrary.hpp"
# include "MapMaster/Tanki/PropMetaData.hpp"

using namespace MapMaster::Tanki;

PropGPUResourceManagerR3DBackend::MeshResource PropGPUResourceManagerR3DBackend::CreateMeshResource (CPUResourceManager::PropMeshResource & meshResource) {
	// #FIXME: aabb must be alive along with the r3d mesh
	MeshResource m = {
		.mesh = std::shared_ptr <R3D_Mesh> (
			new R3D_Mesh (R3D_LoadMesh (R3D_PrimitiveType::R3D_PRIMITIVE_TRIANGLES, * meshResource.mesh, & meshResource.aabb)),
			unloadMeshResource
		),
		.meta = meshResource.meta,
	};

	return m;
}

PropGPUResourceManagerR3DBackend::TextureResource PropGPUResourceManagerR3DBackend::CreateTextureResource (const CPUResourceManager::PropTextureResource & textureResource) {
	TextureResource t = {};

	int pixelFormat = PIXELFORMAT_UNCOMPRESSED_R8G8B8;

	if (4 == textureResource.meta.channels) {
		pixelFormat = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
	}

	t.texture = std::shared_ptr <Texture2D> (
		new Texture2D (R3D_LoadTextureFromImage (
			{
				.data = static_cast <void *> (textureResource.pixBuffer.get ()),
				.width = textureResource.meta.width,
				.height = textureResource.meta.height,
				.mipmaps = 1,
				.format = pixelFormat,
			},
			true
		)),
		unloadTextureResource
	);

	GenTextureMipmaps (t.texture.get ());
	SetTextureFilter (* t.texture, TEXTURE_FILTER_TRILINEAR);

	t.meta = textureResource.meta;

	return t;
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

PropGPUResourceManagerR3DBackend::SpriteInfo PropGPUResourceManagerR3DBackend::CreateSpriteInfo (const PropLibrary::PropSprite & sprite, const PropMetaData::Texture & meta) {
	const Vector2 size = {
		.x = static_cast <float> (meta.width * sprite.scale),
		.y = static_cast <float> (meta.height * sprite.scale),
	};

	return {
		.origin = {
			.x = static_cast <float> (sprite.originX * size.x),
			.y = static_cast <float> ((1 - sprite.originY) * size.y),
		},
		.size = size,
	};
}

namespace MapMaster::Tanki {
template class PropGPUResourceManager <PropGPUResourceManagerR3DBackend>;
}  // namespace MapMaster::Tanki

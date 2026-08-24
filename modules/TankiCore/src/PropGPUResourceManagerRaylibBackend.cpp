# include "MapMaster/Tanki/PropGPUResourceManagerRaylibBackend.hpp"

# include <memory>
# include <vector>

# include <raylib.h>

# include "MapMaster/Tanki/PropGPUResourceManager.hpp"
# include "MapMaster/Tanki/PropGPUResourceManager.inl" // IWYU pragma: keep

using namespace MapMaster::Tanki;

template PropGPUResourceManagerRaylibBackend::MeshResource
PropGPUResourceManagerRaylibBackend::CreateMeshResource <true> (
	CPUResourceManager::PropMeshResource &
);

template PropGPUResourceManagerRaylibBackend::MeshResource
PropGPUResourceManagerRaylibBackend::CreateMeshResource <false> (
	CPUResourceManager::PropMeshResource &
);

template PropGPUResourceManagerRaylibBackend::TextureResource
PropGPUResourceManagerRaylibBackend::CreateTextureResource <true> (
	const CPUResourceManager::PropTextureResource &
);

template PropGPUResourceManagerRaylibBackend::TextureResource
PropGPUResourceManagerRaylibBackend::CreateTextureResource <false> (
	const CPUResourceManager::PropTextureResource &
);

template <bool Upload>
PropGPUResourceManagerRaylibBackend::MeshResource PropGPUResourceManagerRaylibBackend::CreateMeshResource (CPUResourceManager::PropMeshResource & meshResource) {
	MeshResource m = {};
	m.mesh = std::shared_ptr <Mesh> (new Mesh {}, unloadMeshResource);

	m.mesh->vertices = meshResource.vertexBuffer.data ();
	m.mesh->vertexCount = static_cast <int> (meshResource.vertexBuffer.size () / 3);
	if (false == meshResource.uvBuffer.empty ()) {
		m.mesh->texcoords = meshResource.uvBuffer.data ();
	}
	if (false == meshResource.normalBuffer.empty ()) {
		m.mesh->normals = meshResource.normalBuffer.data ();
	}

	m.mesh->triangleCount = static_cast <int> (meshResource.indexBuffer.size () / 3);

	m.mesh->indices = meshResource.indexBuffer.data ();

	if constexpr (true == Upload) {
		UploadMesh (m.mesh.get (), false);
	}

	return m;
}

template <bool Upload>
// cppcheck-suppress functionStatic
PropGPUResourceManagerRaylibBackend::TextureResource PropGPUResourceManagerRaylibBackend::CreateTextureResource (const CPUResourceManager::PropTextureResource & textureResource) {
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
		t.texture = std::shared_ptr <Texture2D> (new Texture2D (LoadTextureFromImage (t.image)), unloadTextureResource);

		GenTextureMipmaps (t.texture.get ());
		SetTextureFilter (* t.texture, TEXTURE_FILTER_TRILINEAR);
	}

	return t;
}

void PropGPUResourceManagerRaylibBackend::UploadMeshResource (MeshResource & meshResource) {
	UploadMesh (meshResource.mesh.get (), false);
}

void PropGPUResourceManagerRaylibBackend::UploadTextureResource (TextureResource & textureResource) {
	textureResource.texture = std::shared_ptr <Texture2D> (new Texture2D (LoadTextureFromImage (textureResource.image)), unloadTextureResource);

	GenTextureMipmaps (textureResource.texture.get ());
	SetTextureFilter (* textureResource.texture, TEXTURE_FILTER_TRILINEAR);
}

void PropGPUResourceManagerRaylibBackend::unloadMeshResource (Mesh * mesh) {
	mesh->vertices = nullptr;
	mesh->vertexCount = 0;
	mesh->texcoords = nullptr;
	mesh->normals = nullptr;

	mesh->triangleCount = 0;

	mesh->indices = nullptr;

	UnloadMesh (* mesh);
	// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
	delete mesh;
}

void PropGPUResourceManagerRaylibBackend::unloadTextureResource (Texture2D * texture) {
	UnloadTexture (* texture);
	// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
	delete texture;
}

PropGPUResourceManagerRaylibBackend::SpriteInfo PropGPUResourceManagerRaylibBackend::CreateSpriteInfo (const PropLibrary::PropSprite & sprite, const CPUResourceManager::PropTextureResource & textureResource) {
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

namespace MapMaster::Tanki {
template class PropGPUResourceManager <PropGPUResourceManagerRaylibBackend>;
}  // namespace MapMaster::Tanki

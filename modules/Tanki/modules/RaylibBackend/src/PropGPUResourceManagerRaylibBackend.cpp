# include "MapMaster/Tanki/PropGPUResourceManagerRaylibBackend.hpp"

# include <memory>
# include <vector>

# include <raylib.h>

# include "MapMaster/Tanki/PropGPUResourceManager.hpp"
# include "MapMaster/Tanki/PropGPUResourceManager.inl" // IWYU pragma: keep
# include "MapMaster/Tanki/PropMetaData.hpp"

using namespace MapMaster::Tanki;

PropGPUResourceManagerRaylibBackend::MeshResource PropGPUResourceManagerRaylibBackend::CreateMeshResource (CPUResourceManager::PropMeshResource & meshResource) {
	MeshResource m = {};
	m.mesh = std::shared_ptr <Mesh> (new Mesh {}, unloadMeshResource);

	m.mesh->vertices = meshResource.vertexBuffer.data ();
	m.mesh->vertexCount = static_cast <int> (meshResource.vertexBuffer.size () / 3);
	if (false == meshResource.uvBuffer.empty ()) {
		m.mesh->texcoords = meshResource.uvBuffer.data ();
	}
	// if (false == meshResource.normalBuffer.empty ()) {
	// 	m.mesh->normals = meshResource.normalBuffer.data ();
	// }

	m.mesh->triangleCount = static_cast <int> (meshResource.indexBuffer.size () / 3);

	m.mesh->indices = meshResource.indexBuffer.data ();

	UploadMesh (m.mesh.get (), false);

	m.meta = meshResource.meta;

	return m;
}

PropGPUResourceManagerRaylibBackend::TextureResource PropGPUResourceManagerRaylibBackend::CreateTextureResource (const CPUResourceManager::PropTextureResource & textureResource) {
	TextureResource t = {};

	int pixelFormat = PIXELFORMAT_UNCOMPRESSED_R8G8B8;

	if (4 == textureResource.meta->channels) {
		pixelFormat = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
	}

	t.texture = std::shared_ptr <Texture2D> (
		new Texture2D (LoadTextureFromImage (
			{
				.data = static_cast <void *> (textureResource.pixBuffer.get ()),
				.width = textureResource.meta->width,
				.height = textureResource.meta->height,
				.mipmaps = 1,
				.format = pixelFormat,
			}
		)),
		unloadTextureResource
	);

	GenTextureMipmaps (t.texture.get ());
	SetTextureFilter (* t.texture, TEXTURE_FILTER_TRILINEAR);

	t.meta = textureResource.meta;

	return t;
}

PropGPUResourceManagerRaylibBackend::SpriteResource PropGPUResourceManagerRaylibBackend::CreateSpriteResource (const CPUResourceManager::PropSpriteResource & spriteResource) {
	return {
		.size = spriteResource.size,
		.origin = {
			.x = spriteResource.origin.x * spriteResource.size.x,
			.y = (1 - spriteResource.origin.y) * spriteResource.size.y,
		},
	};
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

namespace MapMaster::Tanki {
template class PropGPUResourceManager <PropGPUResourceManagerRaylibBackend>;
}  // namespace MapMaster::Tanki

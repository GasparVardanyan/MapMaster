# include "MapMaster/Tanki/PropResourceManagerRaylibAdapter.hpp"

# include <memory>

# include <raylib.h>

# include "MapMaster/Tanki/PropResourceManager.hpp"

using namespace MapMaster::Tanki;



PropResourceManagerRaylibAdapter::MeshResource PropResourceManagerRaylibAdapter::loadMeshResource (PropResourceManager::PropMeshResource & meshResource) {
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

	UploadMesh (m.mesh.get (), false);

	return m;
}

// cppcheck-suppress functionStatic
PropResourceManagerRaylibAdapter::TextureResource PropResourceManagerRaylibAdapter::loadTextureResource (const PropResourceManager::PropTextureResource & textureResource) {
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
	t.texture = std::shared_ptr <Texture2D> (new Texture2D (LoadTextureFromImage (t.image)), unloadTextureResource);

	GenTextureMipmaps (t.texture.get ());
	SetTextureFilter (* t.texture, TEXTURE_FILTER_TRILINEAR);

	return t;
}

void PropResourceManagerRaylibAdapter::unloadMeshResource (Mesh * mesh) {
	mesh->vertices = nullptr;
	mesh->vertexCount = 0;
	mesh->texcoords = nullptr;
	mesh->normals = nullptr;

	mesh->triangleCount = 0;

	mesh->indices = nullptr;

	UnloadMesh (* mesh);
	delete mesh;
}

void PropResourceManagerRaylibAdapter::unloadTextureResource (Texture2D * texture) {
	UnloadTexture (* texture);
	// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
	delete texture;
}

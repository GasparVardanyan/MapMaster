# include "PropRaylibResourceManager.hpp"

# include <map>
# include <string>
# include <utility>

# include "Map.hpp"
# include "PropLibrary.hpp"
# include "PropResourceManager.hpp"

Texture2D LoadTextureStb(const PropResourceManager::PropTextureResource & res) {

    Image image = {
        .data = static_cast <void *> (res.pixBuffer.get ()),
        .width = res.width,
        .height = res.height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    Texture2D texture = LoadTextureFromImage (image);

	GenTextureMipmaps(&texture);
	SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);

	return texture;
}

static constexpr float scale = 0.01F;

void PropRaylibResourceManager::loadLibrary (const std::string & path) {
	PropLibrary library;
	library.loadDirectory (path);
	m_resourceManager.addPropLibrary (std::move (library));
}

PropResourceManager & PropRaylibResourceManager::resourceManager () {
	return m_resourceManager;
}

void PropRaylibResourceManager::loadMapLibraries (const Map & map, const std::string & libraryRootDir) {
	for (const auto & [libraryName, groupData] : map.mapObjects ()) {
		loadLibrary (libraryRootDir + "/" + libraryName);
	}
}

void PropRaylibResourceManager::loadMapResources (const Map & map) {
	m_resourceManager.loadMapResources (map);

	const auto & libraries = m_resourceManager.propLibraries ();

	for (const auto & [libraryName, groups] : map.mapObjects ()) {
		const auto & library = libraries.at (libraryName);
		for (const auto & [groupName, props] : groups) {
			const auto & group = library.groups ().at (groupName);
			for (const auto & [propName, propInfo] : props) {
				if (true == group.meshes.contains (propName)) {
					PropResourceManager::PropMeshResource & meshResource = const_cast <PropResourceManager::PropMeshResource &> (m_resourceManager.getMeshResource (libraryName, groupName, propName));
					std::string meshFile = m_resourceManager.propLibraries ().at (libraryName).groups ().at (groupName).meshes.at (propName).file;
					if (false == m_meshResources.contains (libraryName) || false == m_meshResources.at (libraryName).contains (meshFile)) {
						RaylibMeshResource & m = m_meshResources [libraryName] [meshFile];

						m.mesh.vertices = meshResource.vertexBuffer.data ();
						m.mesh.vertexCount = static_cast <int> (meshResource.vertexBuffer.size () / 3);
						if (false == meshResource.uvBuffer.empty ()) {
							m.mesh.texcoords = meshResource.uvBuffer.data ();
						}
						if (false == meshResource.normalBuffer.empty ()) {
							m.mesh.normals = meshResource.normalBuffer.data ();
						}

						m.mesh.triangleCount = static_cast <int> (meshResource.indexBuffer.size () / 3);

						m.mesh.indices = meshResource.indexBuffer.data();

						UploadMesh (& m.mesh, false);
						m.model = LoadModelFromMesh (m.mesh);
					}
					for (const auto & prop : propInfo) {
						std::string textureName = prop.textureName;
						std::string textureFile;
						if (true == textureName.empty ()) {
							textureFile = meshResource.textureFile;
						}
						else {
							textureFile = m_resourceManager.propLibraries ().at (libraryName).groups ().at (groupName).meshes.at (propName).textures.at (textureName);
						}

						textureFile = m_resourceManager.propLibraries ().at (libraryName).actualTextureFile (textureFile);

						if (false == m_textureResources.contains (libraryName) || false == m_textureResources.at (libraryName).contains (textureFile)) {
							const PropResourceManager::PropTextureResource & textureResource = m_resourceManager.getTextureResource (libraryName, groupName, propName, textureName);
							m_textureResources [libraryName] [textureFile] = {
								.texture = LoadTextureStb (textureResource)
							};
						}
					}

					// if (false == textures.contains (library) || false == textures.at (libraryName).contains ())
				}
				else if (true == group.sprites.contains (propName)) {
					const auto & sprite = group.sprites.at (propName);
					std::string textureFile = m_resourceManager.propLibraries ().at (libraryName).actualTextureFile (sprite.diffuseFile);
					if (false == m_textureResources.contains (libraryName) || false == m_textureResources.at (libraryName).contains (textureFile)) {
						auto const & res = m_resourceManager.getTextureResource (libraryName, groupName, propName);
						m_textureResources [libraryName] [textureFile] = {
							.texture = LoadTextureStb (res)
						};
					}

					const RaylibTextureResource & textureResource = m_textureResources [libraryName] [textureFile];
					if (false == m_spriteInfos.contains (libraryName) || false == m_spriteInfos.at (libraryName).contains (textureFile)) { // FIXME: use propName since theoretically multiple sprites can use the same file with different origins and scales
						const Vector2 size = {
							static_cast <float> (textureResource.texture.width * sprite.scale * scale),
							static_cast <float> (textureResource.texture.height * sprite.scale * scale),
						};

						m_spriteInfos [libraryName] [textureFile] = {
							.origin = {
								static_cast <float> (sprite.originX * size.x),
								static_cast <float> ((1 - sprite.originY) * size.y),
							},
							.size = size,
						};
					}
				}
			}
		}
	}
}

const std::map <std::string, std::map <std::string, PropRaylibResourceManager::RaylibMeshResource>> & PropRaylibResourceManager::meshResources () const {
	return m_meshResources;

}
const std::map <std::string, std::map <std::string, PropRaylibResourceManager::RaylibTextureResource>> & PropRaylibResourceManager::textureResources () const {
	return m_textureResources;
}

const std::map <std::string, std::map <std::string, PropRaylibResourceManager::RaylibSpriteInfo>> & PropRaylibResourceManager::spriteInfos () const {
	return m_spriteInfos;
}

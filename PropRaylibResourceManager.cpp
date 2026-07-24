# include "PropRaylibResourceManager.hpp"

# include <algorithm>
# include <condition_variable>
# include <cstddef>
# include <execution>
# include <map>
# include <mutex>
# include <queue>
# include <stack>
# include <string>
# include <thread>
# include <utility>
# include <vector>

# include <raylib.h>

# include "Map.hpp"
# include "PropLibrary.hpp"
# include "PropResourceManager.hpp"



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

void PropRaylibResourceManager::loadMapResources_OLD (const Map & map) {
	m_resourceManager.loadMapResources (map);

	const auto & libraries = m_resourceManager.propLibraries ();

	std::vector <std::pair <std::string, std::string>> meshDescriptors;
	std::vector <std::pair <std::string, std::string>> textureDescriptors;
	std::vector <std::pair <std::string, std::string>> spriteDescriptors;

	for (const auto & [libraryName, groups] : map.mapObjects ()) {
		const auto & library = libraries.at (libraryName);
		for (const auto & [groupName, props] : groups) {
			const auto & group = library.groups ().at (groupName);
			for (const auto & [propName, propInfo] : props) {
				if (true == group.meshes.contains (propName)) {
					std::string meshFile = m_resourceManager.propLibraries ().at (libraryName).groups ().at (groupName).meshes.at (propName).file;
					PropResourceManager::PropMeshResource & meshResource = const_cast <PropResourceManager::PropMeshResource &> (m_resourceManager.propMeshResources ().at (libraryName).at (meshFile));

					meshDescriptors.emplace_back (libraryName, meshFile);

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

						textureDescriptors.emplace_back (libraryName, textureFile);
					}
				}
				else if (true == group.sprites.contains (propName)) {
					const auto & sprite = group.sprites.at (propName);
					std::string textureFile = m_resourceManager.propLibraries ().at (libraryName).actualTextureFile (sprite.diffuseFile);
					textureDescriptors.emplace_back (libraryName, textureFile);
					spriteDescriptors.emplace_back (libraryName, textureFile);

					const PropResourceManager::PropTextureResource & textureResource = m_resourceManager.propTextureResources ().at (libraryName).at (textureFile);
					if (false == m_spriteInfos.contains (libraryName) || false == m_spriteInfos.at (libraryName).contains (textureFile)) { // FIXME: use propName since theoretically multiple sprites can use the same file with different origins and scales
						const Vector2 size = {
							.x = static_cast <float> (textureResource.width * sprite.scale),
							.y = static_cast <float> (textureResource.height * sprite.scale),
						};

						m_spriteInfos [libraryName] [textureFile] = {
							.origin = {
								.x = static_cast <float> (sprite.originX * size.x),
								.y = static_cast <float> ((1 - sprite.originY) * size.y),
							},
							.size = size,
						};
					}
				}
			}
		}
	}

	std::sort (std::execution::par_unseq, meshDescriptors.begin (), meshDescriptors.end ());
	meshDescriptors.erase (std::unique (std::execution::par_unseq, meshDescriptors.begin (), meshDescriptors.end ()), meshDescriptors.end ());
	std::sort (std::execution::par_unseq, textureDescriptors.begin (), textureDescriptors.end ());
	textureDescriptors.erase (std::unique (std::execution::par_unseq, textureDescriptors.begin (), textureDescriptors.end ()), textureDescriptors.end ());

	loadMeshResources (meshDescriptors);
	loadTextureResources (textureDescriptors);

	for (auto & [libraryName, textureFiles] : m_textureResources) {
		for (auto & [textureFile, textureResource] : textureFiles) {
			textureResource.texture = LoadTextureFromImage (textureResource.image);
			GenTextureMipmaps (&textureResource.texture);
			SetTextureFilter (textureResource.texture, TEXTURE_FILTER_TRILINEAR);
		}
	}

	for (auto & [libraryName, meshFiles] : m_meshResources) {
		for (auto & [meshFile, meshResource] : meshFiles) {
			UploadMesh (& meshResource.mesh, false);
			meshResource.model = LoadModelFromMesh (meshResource.mesh);
		}
	}

	// m_resourceManager.dropResources ();
}

void PropRaylibResourceManager::loadMapResources (const Map & map) {
	std::queue <std::pair <std::string, std::string>> meshQueue;
	std::queue <std::pair <std::string, std::string>> textureQueue;

	bool meshesFinished = false;
	bool texturesFinished = false;

	std::mutex meshResourceMutex;
	std::condition_variable meshResourceNotifier;
	std::mutex textureResourceMutex;
	std::condition_variable textureResourceNotifier;

	m_resourceManager.setMeshResourceLoadCallback ([& meshResourceMutex, & meshQueue, & meshResourceNotifier] (const std::string & libraryName, const std::string & meshFile) -> void {
		{
			std::scoped_lock <std::mutex> meshResourceLock (meshResourceMutex);;
			meshQueue.emplace (libraryName, meshFile);
		}

		meshResourceNotifier.notify_one ();
	});

	m_resourceManager.setTextureResourceLoadCallback ([& textureResourceMutex, & textureQueue, & textureResourceNotifier] (const std::string & libraryName, const std::string & textureFile) -> void {
		{
			std::scoped_lock <std::mutex> textureResourceLock (textureResourceMutex);
			textureQueue.emplace (libraryName, textureFile);
		}

		textureResourceNotifier.notify_one ();
	});

	m_resourceManager.setMapMeshResourcesLoadCallback ([& meshesFinished, & meshResourceMutex, & meshResourceNotifier] () -> void {
		{
			std::scoped_lock <std::mutex> meshResourceLock (meshResourceMutex);
			meshesFinished = true;
		}

		meshResourceNotifier.notify_one ();
	});

	m_resourceManager.setMapTextureResourcesLoadCallback ([&texturesFinished, &textureResourceMutex, & textureResourceNotifier] () -> void {
		{
			std::scoped_lock <std::mutex> textureResourceLock (textureResourceMutex);
			texturesFinished = true;
		}

		textureResourceNotifier.notify_one ();
	});

	std::thread resLoaderThread ([this, & map] () -> void {
		m_resourceManager.loadMapResources (map);
	});

	while (true) {
		std::stack <std::pair <std::string, std::string>> meshesToProcess;
		bool finished = false;

		{
			std::unique_lock <std::mutex> meshResourceLock (meshResourceMutex);
			meshResourceNotifier.wait (meshResourceLock, [& meshQueue, & meshesFinished] () -> bool {
				return false == meshQueue.empty () || true == meshesFinished;
			});

			while (false == meshQueue.empty ()) {
				meshesToProcess.push (meshQueue.front ());
				meshQueue.pop ();
			}

			finished = meshesFinished;
		}

		while (false == meshesToProcess.empty ()) {
			std::pair <std::string, std::string> meshDescriptor = meshesToProcess.top ();
			meshesToProcess.pop ();

			PropResourceManager::PropMeshResource & res = const_cast <PropResourceManager::PropMeshResource &> (
				m_resourceManager.propMeshResources ().at (meshDescriptor.first).at (meshDescriptor.second)
			);
			m_meshResources [meshDescriptor.first] [meshDescriptor.second] = loadMeshResources (res);

			RaylibMeshResource & meshResource = m_meshResources.at (meshDescriptor.first).at (meshDescriptor.second);
			UploadMesh (& meshResource.mesh, false);
			meshResource.model = LoadModelFromMesh (meshResource.mesh);
		}

		if (true == finished) {
			break;
		}
	}

	while (true) {
		std::stack <std::pair <std::string, std::string>> texturesToProcess;
		bool finished = false;

		{
			std::unique_lock <std::mutex> textureResourceLock (textureResourceMutex);
			textureResourceNotifier.wait (textureResourceLock, [& textureQueue, & texturesFinished] () -> bool {
				return false == textureQueue.empty () || true == texturesFinished;
			});

			while (false == textureQueue.empty ()) {
				texturesToProcess.push (textureQueue.front ());
				textureQueue.pop ();
			}

			finished = texturesFinished;
		}

		while (false == texturesToProcess.empty ()) {
			std::pair <std::string, std::string> textureDescriptor = texturesToProcess.top ();
			texturesToProcess.pop ();

			const PropResourceManager::PropTextureResource & res = m_resourceManager.propTextureResources ().at (textureDescriptor.first).at (textureDescriptor.second);
			m_textureResources [textureDescriptor.first] [textureDescriptor.second] = loadTextureResources (res);

			RaylibTextureResource & textureResource = m_textureResources.at (textureDescriptor.first).at (textureDescriptor.second);
			textureResource.texture = LoadTextureFromImage (textureResource.image);
			GenTextureMipmaps (&textureResource.texture);
			SetTextureFilter (textureResource.texture, TEXTURE_FILTER_TRILINEAR);
		}

		if (true == finished) {
			break;
		}
	}

	resLoaderThread.join ();
	m_resourceManager.clearCallbacks ();

	const auto & libraries = m_resourceManager.propLibraries ();

	std::vector <std::pair <std::string, std::string>> spriteDescriptors;

	for (const auto & [libraryName, groups] : map.mapObjects ()) {
		const auto & library = libraries.at (libraryName);
		for (const auto & [groupName, props] : groups) {
			const auto & group = library.groups ().at (groupName);
			for (const auto & [propName, propInfo] : props) {
				if (true == group.sprites.contains (propName)) {
					const auto & sprite = group.sprites.at (propName);
					std::string textureFile = m_resourceManager.propLibraries ().at (libraryName).actualTextureFile (sprite.diffuseFile);
					spriteDescriptors.emplace_back (libraryName, textureFile);

					const PropResourceManager::PropTextureResource & textureResource = m_resourceManager.propTextureResources ().at (libraryName).at (textureFile);
					if (false == m_spriteInfos.contains (libraryName) || false == m_spriteInfos.at (libraryName).contains (textureFile)) { // FIXME: use propName since theoretically multiple sprites can use the same file with different origins and scales
						const Vector2 size = {
							.x = static_cast <float> (textureResource.width * sprite.scale),
							.y = static_cast <float> (textureResource.height * sprite.scale),
						};

						m_spriteInfos [libraryName] [textureFile] = {
							.origin = {
								.x = static_cast <float> (sprite.originX * size.x),
								.y = static_cast <float> ((1 - sprite.originY) * size.y),
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

// cppcheck-suppress functionStatic
PropRaylibResourceManager::RaylibMeshResource PropRaylibResourceManager::loadMeshResources (PropResourceManager::PropMeshResource & meshResource) {
	RaylibMeshResource m = {};

	m.mesh.vertices = meshResource.vertexBuffer.data ();
	m.mesh.vertexCount = static_cast <int> (meshResource.vertexBuffer.size () / 3);
	if (false == meshResource.uvBuffer.empty ()) {
		m.mesh.texcoords = meshResource.uvBuffer.data ();
	}
	if (false == meshResource.normalBuffer.empty ()) {
		m.mesh.normals = meshResource.normalBuffer.data ();
	}

	m.mesh.triangleCount = static_cast <int> (meshResource.indexBuffer.size () / 3);

	m.mesh.indices = meshResource.indexBuffer.data ();

	return m;
}

// cppcheck-suppress functionStatic
PropRaylibResourceManager::RaylibTextureResource PropRaylibResourceManager::loadTextureResources (const PropResourceManager::PropTextureResource & textureResource) {
	int pixelFormat = PIXELFORMAT_UNCOMPRESSED_R8G8B8;

	if (4 == textureResource.channels) {
		pixelFormat = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
	}

	Image image = {
		.data = static_cast <void *> (textureResource.pixBuffer.get ()),
		.width = textureResource.width,
		.height = textureResource.height,
		.mipmaps = 1,
		.format = pixelFormat
	};

	return {
		.image = image,
		.texture = {}
	};
}

void PropRaylibResourceManager::loadMeshResources (const std::vector <std::pair <std::string, std::string>> & meshDescriptors) {
	std::vector <RaylibMeshResource> resources;
	resources.resize (meshDescriptors.size ());

	std::transform (
		std::execution::par_unseq,
		meshDescriptors.cbegin (),
		meshDescriptors.cend (),
		resources.begin (),
		[this] (const std::pair <std::string, std::string> & descriptor) {
			PropResourceManager::PropMeshResource & res = const_cast <PropResourceManager::PropMeshResource &> (m_resourceManager.propMeshResources ().at (descriptor.first).at (descriptor.second));
			return loadMeshResources (res);
		}
	);

	std::size_t mI = 0;

	for (const std::pair <std::string, std::string> & descriptor : meshDescriptors) {
		// NOLINTNEXTLINE(hicpp-move-const-arg,performance-move-const-arg)
		m_meshResources [descriptor.first] [descriptor.second] = std::move (resources [mI]);
		mI++;
	}
}

void PropRaylibResourceManager::loadTextureResources (const std::vector <std::pair <std::string, std::string>> & textureDescriptors) {
	std::vector <RaylibTextureResource> resources;
	resources.resize (textureDescriptors.size ());

	std::transform (
		std::execution::par_unseq,
		textureDescriptors.cbegin (),
		textureDescriptors.cend (),
		resources.begin (),
		[this] (const std::pair <std::string, std::string> & descriptor) {
			const PropResourceManager::PropTextureResource & res = m_resourceManager.propTextureResources ().at (descriptor.first).at (descriptor.second);
			return loadTextureResources (res);
		}
	);

	std::size_t tI = 0;
	for (const std::pair <std::string, std::string> & descriptor : textureDescriptors) {
		// NOLINTNEXTLINE(hicpp-move-const-arg,performance-move-const-arg)
		m_textureResources [descriptor.first] [descriptor.second] = std::move (resources [tI]);
		tI++;
	}
}

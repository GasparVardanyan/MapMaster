# include "MapMaster/Tanki/RaylibPropResourceManager.hpp"

# include <algorithm>
# include <condition_variable>
# include <cstddef>
# include <execution>
# include <map>
# include <memory>
# include <mutex>
# include <queue>
# include <stack>
# include <string>
# include <thread>
# include <tuple>
# include <utility>
# include <vector>

# include <raylib.h>

# include "MapMaster/Tanki/Map.hpp"
# include "MapMaster/Tanki/PropLibrary.hpp"
# include "MapMaster/Tanki/PropResourceManager.hpp"



RaylibPropResourceManager::RaylibPropResourceManager (bool parseCollisionPrimitives)
	: m_resourceManager (parseCollisionPrimitives)
{
}

void RaylibPropResourceManager::loadLibrary (const std::string & path) {
	std::shared_ptr <PropLibrary> library = std::make_shared <PropLibrary> ();
	library->loadDirectory (path);
	m_resourceManager.addPropLibrary (std::move (library));
}

void RaylibPropResourceManager::loadMapLibraries (const Map & map, const std::string & libraryRootDir) {
	for (const auto & [libraryName, groupData] : map.mapObjects ()) {
		loadLibrary (libraryRootDir + "/" + libraryName);
	}
}




//  _      ____          _____  ______ _____   _____
// | |    / __ \   /\   |  __ \|  ____|  __ \ / ____|
// | |   | |  | | /  \  | |  | | |__  | |__) | (___
// | |   | |  | |/ /\ \ | |  | |  __| |  _  / \___ \
// | |___| |__| / ____ \| |__| | |____| | \ \ ____) |
// |______\____/_/    \_\_____/|______|_|  \_\_____/
//

void RaylibPropResourceManager::loadMapResources (const Map & map) {
	// TODO: use tuple
	std::queue <std::tuple <std::string, std::string, std::shared_ptr <PropResourceManager::PropMeshResource>>> meshQueue;
	std::queue <std::tuple <std::string, std::string, std::shared_ptr <PropResourceManager::PropTextureResource>>> textureQueue;

	bool meshesFinished = false;
	bool texturesFinished = false;

	std::mutex meshResourceMutex;
	std::condition_variable meshResourceNotifier;
	std::mutex textureResourceMutex;
	std::condition_variable textureResourceNotifier;

	m_resourceManager.setMeshResourceLoadCallback ([& meshResourceMutex, & meshQueue, & meshResourceNotifier] (const std::string & libraryName, const std::string & meshFile, std::shared_ptr <PropResourceManager::PropMeshResource> meshResource, std::shared_ptr <PropResourceManager::Collider>) -> void {
		{
			std::scoped_lock <std::mutex> meshResourceLock (meshResourceMutex);;
			meshQueue.emplace (libraryName, meshFile, std::move (meshResource));
		}

		meshResourceNotifier.notify_one ();
	});

	m_resourceManager.setTextureResourceLoadCallback ([& textureResourceMutex, & textureQueue, & textureResourceNotifier] (const std::string & libraryName, const std::string & textureFile, std::shared_ptr <PropResourceManager::PropTextureResource> textureResource) -> void {
		{
			std::scoped_lock <std::mutex> textureResourceLock (textureResourceMutex);
			textureQueue.emplace  (libraryName, textureFile, std::move (textureResource));
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

	m_resourceManager.setMapTextureResourcesLoadCallback ([& texturesFinished, &textureResourceMutex, & textureResourceNotifier] () -> void {
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
		std::stack <std::tuple <std::string, std::string, std::shared_ptr <PropResourceManager::PropMeshResource>>> meshesToProcess;
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
			const auto & [libraryName, meshFile, meshRes] = meshesToProcess.top ();

			RaylibMeshResource & meshResource = m_meshResources [libraryName] [meshFile];
			meshResource = loadMeshResource (const_cast <PropResourceManager::PropMeshResource &> (
				* meshRes.get ()
			));

			UploadMesh (meshResource.mesh.get (), false);

			meshesToProcess.pop ();
		}

		if (true == finished) {
			break;
		}
	}

	while (true) {
		std::stack <std::tuple <std::string, std::string, std::shared_ptr <PropResourceManager::PropTextureResource>>> texturesToProcess;
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
			const auto & [libraryName, textureFile, textureRes] = texturesToProcess.top ();

			RaylibTextureResource & textureResource = m_textureResources [libraryName] [textureFile];
			textureResource = loadTextureResource (* textureRes.get ());

			textureResource.texture = std::shared_ptr <Texture2D> (new Texture2D (LoadTextureFromImage (textureResource.image)), unloadTextureResource);

			GenTextureMipmaps (textureResource.texture.get ());
			SetTextureFilter (* textureResource.texture, TEXTURE_FILTER_TRILINEAR);

			texturesToProcess.pop ();
		}

		if (true == finished) {
			break;
		}
	}

	resLoaderThread.join ();
	m_resourceManager.clearCallbacks ();

	const std::map <std::string, std::shared_ptr <PropLibrary>> & libraries = m_resourceManager.propLibraries ();
	// cppcheck-suppress shadowFunction
	const std::map <std::string, std::map <std::string, std::shared_ptr <PropResourceManager::PropTextureResource>>> & textureResources = m_resourceManager.propTextureResources ();

	std::vector <std::pair <std::string, std::string>> spriteDescriptors;

	for (const auto & [libraryName, groups] : map.mapObjects ()) {
		const PropLibrary & library = * libraries.at (libraryName);
		const std::map <std::string, PropLibrary::Group> & libraryGroups = library.groups ();
		for (const auto & [groupName, props] : groups) {
			const PropLibrary::Group & group = libraryGroups.at (groupName);
			for (const auto & [propName, propInfo] : props) {
				if (true == group.sprites.contains (propName)) {
					const auto & sprite = group.sprites.at (propName);
					std::string textureFile = library.getActualTextureFileName (sprite.diffuseFile);
					spriteDescriptors.emplace_back (libraryName, textureFile);

					const PropResourceManager::PropTextureResource & textureResource = * textureResources.at (libraryName).at (textureFile);
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

void RaylibPropResourceManager::loadMapResources_OLD (const Map & map) {
	m_resourceManager.loadMapResources (map);

	const std::map <std::string, std::shared_ptr <PropLibrary>> & libraries = m_resourceManager.propLibraries ();
	// cppcheck-suppress shadowFunction
	const std::map <std::string, std::map <std::string, std::shared_ptr <PropResourceManager::PropTextureResource>>> & textureResources = m_resourceManager.propTextureResources ();

	std::vector <std::pair <std::string, std::string>> meshDescriptors;
	std::vector <std::pair <std::string, std::string>> textureDescriptors;
	std::vector <std::pair <std::string, std::string>> spriteDescriptors;

	for (const auto & [libraryName, groups] : map.mapObjects ()) {
		const PropLibrary & library = * libraries.at (libraryName);
		const std::map <std::string, std::shared_ptr <PropResourceManager::PropTextureResource>> & libraryTextureResources = textureResources.at (libraryName);
		const std::map <std::string, PropLibrary::Group> & libraryGroups = library.groups ();
		for (const auto & [groupName, props] : groups) {
			const PropLibrary::Group & group = libraryGroups.at (groupName);
			for (const auto & [propName, propInfo] : props) {
				if (true == group.meshes.contains (propName)) {
					std::string meshFile = group.meshes.at (propName).file;
					const PropResourceManager::PropMeshResource & meshResource = const_cast <PropResourceManager::PropMeshResource &> (* m_resourceManager.propMeshResources ().at (libraryName).at (meshFile));

					meshDescriptors.emplace_back (libraryName, meshFile);

					for (const auto & prop : propInfo) {
						std::string textureName = prop.textureName;
						std::string textureFile;
						if (true == textureName.empty ()) {
							textureFile = meshResource.textureFile;
						}
						else {
							textureFile = group.meshes.at (propName).textures.at (textureName);
						}

						textureFile = library.getActualTextureFileName (textureFile);

						textureDescriptors.emplace_back (libraryName, textureFile);
					}
				}
				else if (true == group.sprites.contains (propName)) {
					const auto & sprite = group.sprites.at (propName);
					std::string textureFile = library.getActualTextureFileName (sprite.diffuseFile);
					textureDescriptors.emplace_back (libraryName, textureFile);
					spriteDescriptors.emplace_back (libraryName, textureFile);

					const PropResourceManager::PropTextureResource & textureResource = * libraryTextureResources.at (textureFile);
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
			textureResource.texture = std::make_shared <Texture2D> (LoadTextureFromImage (textureResource.image));
			GenTextureMipmaps (textureResource.texture.get ());
			SetTextureFilter (* textureResource.texture, TEXTURE_FILTER_TRILINEAR);
		}
	}

	for (auto & [libraryName, meshFiles] : m_meshResources) {
		for (auto & [meshFile, meshResource] : meshFiles) {
			UploadMesh (meshResource.mesh.get (), false);
		}
	}
}

void RaylibPropResourceManager::loadMeshResources (const std::vector <std::pair <std::string, std::string>> & meshDescriptors) {
	std::vector <RaylibMeshResource> resources;
	resources.resize (meshDescriptors.size ());

	std::transform (
		std::execution::par_unseq,
		meshDescriptors.cbegin (),
		meshDescriptors.cend (),
		resources.begin (),
		[this] (const std::pair <std::string, std::string> & descriptor) {
			// NOLINTNEXTLINE(hicpp-use-auto,modernize-use-auto)
			return loadMeshResource (
				const_cast <PropResourceManager::PropMeshResource &> (
					* m_resourceManager.propMeshResources ().at (descriptor.first).at (descriptor.second)
				)
			);
		}
	);

	std::size_t mI = 0;

	for (const auto & [libraryName, meshFile] : meshDescriptors) {
		// NOLINTNEXTLINE(hicpp-move-const-arg,performance-move-const-arg)
		m_meshResources [libraryName] [meshFile] = std::move (resources [mI]);
		mI++;
	}
}

void RaylibPropResourceManager::loadTextureResources (const std::vector <std::pair <std::string, std::string>> & textureDescriptors) {
	std::vector <RaylibTextureResource> resources;
	resources.resize (textureDescriptors.size ());

	std::transform (
		std::execution::par_unseq,
		textureDescriptors.cbegin (),
		textureDescriptors.cend (),
		resources.begin (),
		[this] (const std::pair <std::string, std::string> & descriptor) {
			return loadTextureResource (* m_resourceManager.propTextureResources ().at (descriptor.first).at (descriptor.second));
		}
	);

	std::size_t tI = 0;
	for (const auto & [libraryName, textureFile] : textureDescriptors) {
		// NOLINTNEXTLINE(hicpp-move-const-arg,performance-move-const-arg)
		m_textureResources [libraryName] [textureFile] = std::move (resources [tI]);
		tI++;
	}
}



//  _      ____          _____  ______ _____    _    _ ______ _      _____  ______ _____   _____
// | |    / __ \   /\   |  __ \|  ____|  __ \  | |  | |  ____| |    |  __ \|  ____|  __ \ / ____|
// | |   | |  | | /  \  | |  | | |__  | |__) | | |__| | |__  | |    | |__) | |__  | |__) | (___
// | |   | |  | |/ /\ \ | |  | |  __| |  _  /  |  __  |  __| | |    |  ___/|  __| |  _  / \___ \
// | |___| |__| / ____ \| |__| | |____| | \ \  | |  | | |____| |____| |    | |____| | \ \ ____) |
// |______\____/_/    \_\_____/|______|_|  \_\ |_|  |_|______|______|_|    |______|_|  \_\_____/
//

// cppcheck-suppress functionStatic
RaylibPropResourceManager::RaylibMeshResource RaylibPropResourceManager::loadMeshResource (PropResourceManager::PropMeshResource & meshResource) {
	RaylibMeshResource m = {};
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

	return m;
}

// cppcheck-suppress functionStatic
RaylibPropResourceManager::RaylibTextureResource RaylibPropResourceManager::loadTextureResource (const PropResourceManager::PropTextureResource & textureResource) {
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

void RaylibPropResourceManager::unloadMeshResource (Mesh * mesh) {
	mesh->vertices = nullptr;
	mesh->vertexCount = 0;
	mesh->texcoords = nullptr;
	mesh->normals = nullptr;

	mesh->triangleCount = 0;

	mesh->indices = nullptr;

	UnloadMesh (* mesh);
	delete mesh;
}

void RaylibPropResourceManager::unloadTextureResource (Texture2D * texture) {
	UnloadTexture (* texture);
	// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
	delete texture;
}



//   _____ ______ _______ _______ ______ _____   _____
//  / ____|  ____|__   __|__   __|  ____|  __ \ / ____|
// | |  __| |__     | |     | |  | |__  | |__) | (___
// | | |_ |  __|    | |     | |  |  __| |  _  / \___ \
// | |__| | |____   | |     | |  | |____| | \ \ ____) |
//  \_____|______|  |_|     |_|  |______|_|  \_\_____/
//

const PropResourceManager & RaylibPropResourceManager::resourceManager () {
	return m_resourceManager;
}

const std::map <std::string, std::map <std::string, RaylibPropResourceManager::RaylibMeshResource>> & RaylibPropResourceManager::meshResources () const {
	return m_meshResources;

}
const std::map <std::string, std::map <std::string, RaylibPropResourceManager::RaylibTextureResource>> & RaylibPropResourceManager::textureResources () const {
	return m_textureResources;
}

const std::map <std::string, std::map <std::string, RaylibPropResourceManager::RaylibSpriteInfo>> & RaylibPropResourceManager::spriteInfos () const {
	return m_spriteInfos;
}

# include "MapMaster/Tanki/PropGPUResourceManager.hpp"

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

# include "MapMaster/Tanki/Map.hpp"
# include "MapMaster/Tanki/PropLibrary.hpp"
# include "MapMaster/Tanki/PropCPUResourceManager.hpp"

namespace MapMaster::Tanki {

template <class PropGPUResourceManagerAdapter>
PropGPUResourceManager <PropGPUResourceManagerAdapter>::PropGPUResourceManager (bool parseCollisionPrimitives)
	: m_resourceManager (parseCollisionPrimitives)
{
}

template <class PropGPUResourceManagerAdapter>
void PropGPUResourceManager <PropGPUResourceManagerAdapter>::loadLibrary (const std::string & path) {
	std::shared_ptr <PropLibrary> library = std::make_shared <PropLibrary> ();
	library->loadDirectory (path);
	m_resourceManager.addPropLibrary (std::move (library));
}

template <class PropGPUResourceManagerAdapter>
void PropGPUResourceManager <PropGPUResourceManagerAdapter>::loadMapLibraries (const Map & map, const std::string & libraryRootDir) {
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

template <class PropGPUResourceManagerAdapter>
void PropGPUResourceManager <PropGPUResourceManagerAdapter>::loadMapResources (const Map & map) {
	// TODO: use tuple
	std::queue <std::tuple <std::string, std::string, std::shared_ptr <typename CPUResourceManager::PropMeshResource>>> meshQueue;
	std::queue <std::tuple <std::string, std::string, std::shared_ptr <typename CPUResourceManager::PropTextureResource>>> textureQueue;

	bool meshesFinished = false;
	bool texturesFinished = false;

	std::mutex meshResourceMutex;
	std::condition_variable meshResourceNotifier;
	std::mutex textureResourceMutex;
	std::condition_variable textureResourceNotifier;

	m_resourceManager.setMeshResourceLoadCallback ([& meshResourceMutex, & meshQueue, & meshResourceNotifier] (const std::string & libraryName, const std::string & meshFile, std::shared_ptr <typename CPUResourceManager::PropMeshResource> meshResource, std::shared_ptr <typename CPUResourceManager::Collider>) -> void {
		{
			std::scoped_lock <std::mutex> meshResourceLock (meshResourceMutex);;
			meshQueue.emplace (libraryName, meshFile, std::move (meshResource));
		}

		meshResourceNotifier.notify_one ();
	});

	m_resourceManager.setTextureResourceLoadCallback ([& textureResourceMutex, & textureQueue, & textureResourceNotifier] (const std::string & libraryName, const std::string & textureFile, std::shared_ptr <typename CPUResourceManager::PropTextureResource> textureResource) -> void {
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
		std::stack <std::tuple <std::string, std::string, std::shared_ptr <typename CPUResourceManager::PropMeshResource>>> meshesToProcess;
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

			m_meshResources [libraryName] [meshFile] = PropGPUResourceManagerAdapter::CreateMeshResource (const_cast <CPUResourceManager::PropMeshResource &> (
				* meshRes.get ()
			));

			meshesToProcess.pop ();
		}

		if (true == finished) {
			break;
		}
	}

	while (true) {
		std::stack <std::tuple <std::string, std::string, std::shared_ptr <typename CPUResourceManager::PropTextureResource>>> texturesToProcess;
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

			m_textureResources [libraryName] [textureFile] = PropGPUResourceManagerAdapter::CreateTextureResource (* textureRes.get ());

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
	const std::map <std::string, std::map <std::string, std::shared_ptr <typename CPUResourceManager::PropTextureResource>>> & textureResources = m_resourceManager.propTextureResources ();

	for (const auto & [libraryName, groups] : map.mapObjects ()) {
		const PropLibrary & library = * libraries.at (libraryName);
		const std::map <std::string, PropLibrary::Group> & libraryGroups = library.groups ();
		for (const auto & [groupName, props] : groups) {
			const PropLibrary::Group & group = libraryGroups.at (groupName);
			for (const auto & [propName, propInfo] : props) {
				if (true == group.sprites.contains (propName)) {
					const PropLibrary::PropSprite & sprite = group.sprites.at (propName);
					std::string textureFile = library.getActualTextureFileName (sprite.diffuseFile);

					const typename CPUResourceManager::PropTextureResource & textureResource = * textureResources.at (libraryName).at (textureFile);

					// FIXME: use propName since theoretically multiple sprites can use the same file with different origins and scales
					if (false == m_spriteInfos.contains (libraryName) || false == m_spriteInfos.at (libraryName).contains (textureFile)) {
						m_spriteInfos [libraryName] [textureFile] = PropGPUResourceManagerAdapter::CreateSpriteInfo (sprite, textureResource);
					}
				}
			}
		}
	}
}

template <class PropGPUResourceManagerAdapter>
void PropGPUResourceManager <PropGPUResourceManagerAdapter>::loadMapResources_OLD (const Map & map) {
	m_resourceManager.loadMapResources (map);

	const std::map <std::string, std::shared_ptr <PropLibrary>> & libraries = m_resourceManager.propLibraries ();
	// cppcheck-suppress shadowFunction
	const std::map <std::string, std::map <std::string, std::shared_ptr <typename CPUResourceManager::PropTextureResource>>> & textureResources = m_resourceManager.propTextureResources ();

	std::vector <std::pair <std::string, std::string>> meshDescriptors;
	std::vector <std::pair <std::string, std::string>> textureDescriptors;
	std::vector <std::pair <std::string, std::string>> spriteDescriptors;

	for (const auto & [libraryName, groups] : map.mapObjects ()) {
		const PropLibrary & library = * libraries.at (libraryName);
		const std::map <std::string, std::shared_ptr <typename CPUResourceManager::PropTextureResource>> & libraryTextureResources = textureResources.at (libraryName);
		const std::map <std::string, PropLibrary::Group> & libraryGroups = library.groups ();
		for (const auto & [groupName, props] : groups) {
			const PropLibrary::Group & group = libraryGroups.at (groupName);
			for (const auto & [propName, propInfo] : props) {
				if (true == group.meshes.contains (propName)) {
					std::string meshFile = group.meshes.at (propName).file;
					const typename CPUResourceManager::PropMeshResource & meshResource = const_cast <CPUResourceManager::PropMeshResource &> (* m_resourceManager.propMeshResources ().at (libraryName).at (meshFile));

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

					const typename CPUResourceManager::PropTextureResource & textureResource = * libraryTextureResources.at (textureFile);

					// FIXME: use propName since theoretically multiple sprites can use the same file with different origins and scales
					if (false == m_spriteInfos.contains (libraryName) || false == m_spriteInfos.at (libraryName).contains (textureFile)) {
						m_spriteInfos [libraryName] [textureFile] = PropGPUResourceManagerAdapter::CreateSpriteInfo (sprite, textureResource);
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
			PropGPUResourceManagerAdapter::UploadTextureResource (textureResource);
		}
	}

	for (auto & [libraryName, meshFiles] : m_meshResources) {
		for (auto & [meshFile, meshResource] : meshFiles) {
			PropGPUResourceManagerAdapter::UploadMeshResource (meshResource);
		}
	}
}

template <class PropGPUResourceManagerAdapter>
void PropGPUResourceManager <PropGPUResourceManagerAdapter>::loadMeshResources (const std::vector <std::pair <std::string, std::string>> & meshDescriptors) {
	std::vector <MeshResource> resources;
	resources.resize (meshDescriptors.size ());

	std::transform (
		std::execution::par_unseq,
		meshDescriptors.cbegin (),
		meshDescriptors.cend (),
		resources.begin (),
		[this] (const std::pair <std::string, std::string> & descriptor) {
			// NOLINTNEXTLINE(hicpp-use-auto,modernize-use-auto)
			return PropGPUResourceManagerAdapter::template CreateMeshResource <false> (
				const_cast <CPUResourceManager::PropMeshResource &> (
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

template <class PropGPUResourceManagerAdapter>
void PropGPUResourceManager <PropGPUResourceManagerAdapter>::loadTextureResources (const std::vector <std::pair <std::string, std::string>> & textureDescriptors) {
	std::vector <TextureResource> resources;
	resources.resize (textureDescriptors.size ());

	std::transform (
		std::execution::par_unseq,
		textureDescriptors.cbegin (),
		textureDescriptors.cend (),
		resources.begin (),
		[this] (const std::pair <std::string, std::string> & descriptor) {
			return PropGPUResourceManagerAdapter::template CreateTextureResource <false> (* m_resourceManager.propTextureResources ().at (descriptor.first).at (descriptor.second));
		}
	);

	std::size_t tI = 0;
	for (const auto & [libraryName, textureFile] : textureDescriptors) {
		// NOLINTNEXTLINE(hicpp-move-const-arg,performance-move-const-arg)
		m_textureResources [libraryName] [textureFile] = std::move (resources [tI]);
		tI++;
	}
}



//   _____ ______ _______ _______ ______ _____   _____
//  / ____|  ____|__   __|__   __|  ____|  __ \ / ____|
// | |  __| |__     | |     | |  | |__  | |__) | (___
// | | |_ |  __|    | |     | |  |  __| |  _  / \___ \
// | |__| | |____   | |     | |  | |____| | \ \ ____) |
//  \_____|______|  |_|     |_|  |______|_|  \_\_____/
//

template <class PropGPUResourceManagerAdapter>
const PropGPUResourceManager <PropGPUResourceManagerAdapter>::CPUResourceManager & PropGPUResourceManager <PropGPUResourceManagerAdapter>::resourceManager () {
	return m_resourceManager;
}

template <class PropGPUResourceManagerAdapter>
const std::map <std::string, std::map <std::string, typename PropGPUResourceManager <PropGPUResourceManagerAdapter>::MeshResource>> & PropGPUResourceManager <PropGPUResourceManagerAdapter>::meshResources () const {
	return m_meshResources;

}
template <class PropGPUResourceManagerAdapter>
const std::map <std::string, std::map <std::string, typename PropGPUResourceManager <PropGPUResourceManagerAdapter>::TextureResource>> & PropGPUResourceManager <PropGPUResourceManagerAdapter>::textureResources () const {
	return m_textureResources;
}

template <class PropGPUResourceManagerAdapter>
const std::map <std::string, std::map <std::string, typename PropGPUResourceManager <PropGPUResourceManagerAdapter>::SpriteInfo>> & PropGPUResourceManager <PropGPUResourceManagerAdapter>::spriteInfos () const {
	return m_spriteInfos;
}

}  // namespace MapMaster::Tanki

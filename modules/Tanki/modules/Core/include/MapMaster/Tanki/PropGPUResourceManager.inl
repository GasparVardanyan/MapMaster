# include "MapMaster/Tanki/PropGPUResourceManager.hpp"

# include <algorithm>
# include <cstddef>
# include <execution>
# include <map>
# include <memory>
# include <string>
# include <thread>
# include <tuple>
# include <utility>
# include <vector>

# include "MapMaster/Tanki/Map.hpp"
# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/PropLibrary.hpp"
# include "MapMaster/Tanki/PropMetaData.hpp"
# include "MapMaster/Tanki/ResourceSettings.hpp"

namespace MapMaster::Tanki {

template <class PropGPUResourceManagerBackend>
PropGPUResourceManager <PropGPUResourceManagerBackend>::PropGPUResourceManager (const ResourceSettings & resourceSettings)
	: m_resourceManager (resourceSettings)
	, m_resourceSettings (resourceSettings)
{
}

template <class PropGPUResourceManagerBackend>
void PropGPUResourceManager <PropGPUResourceManagerBackend>::loadLibrary (const std::string & path) {
	std::shared_ptr <PropLibrary> library = std::make_shared <PropLibrary> ();
	library->loadDirectory (path);
	m_resourceManager.addPropLibrary (library);

	const std::string & libraryName = library->name ();

	m_propLibraries.try_emplace (
		libraryName,
		std::move (library)
	);
}

template <class PropGPUResourceManagerBackend>
void PropGPUResourceManager <PropGPUResourceManagerBackend>::loadMapLibraries (const Map & map, const std::string & libraryRootDir) {
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

template <class PropGPUResourceManagerBackend>
void PropGPUResourceManager <PropGPUResourceManagerBackend>::loadMapResources (const Map & map) {
	std::thread resLoaderThread ([this, & map] () -> void {
		m_resourceManager.loadMapResources (map);
	});

	m_resourceManager.meshLoader ().listen ([this] (std::vector <std::tuple <
		std::string,
		std::string,
		std::shared_ptr <typename CPUResourceManager::PropMeshResource>
	// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
	>> && meshesToProcess) -> void {
		for (auto & [libraryName, meshFile, meshRes] : meshesToProcess) {
			m_meshResources [std::move (libraryName)] [std::move (meshFile)] = PropGPUResourceManagerBackend::CreateMeshResource (const_cast <CPUResourceManager::PropMeshResource &> (
				* meshRes
			));
		}
	});

	m_resourceManager.textureLoader ().listen ([this] (std::vector <std::tuple <
		std::string,
		std::string,
		std::string,
		std::shared_ptr <typename CPUResourceManager::PropTextureResource>
	// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
	>> && texturesToProcess) -> void {
		for (auto & [libraryName, textureFile, _, textureRes] : texturesToProcess) {
			m_textureResources [std::move (libraryName)] [std::move (textureFile)] = PropGPUResourceManagerBackend::CreateTextureResource (
				* textureRes
			);
		}
	});

	m_resourceManager.spriteGenerator ().listen ([this] (std::vector <std::tuple <
		std::string,
		std::string,
		PropMetaData::Sprite,
		std::shared_ptr <typename CPUResourceManager::PropSpriteResource>
	>> && spritesToProcess) -> void {
		for (auto & [libraryName, textureFile, _, spriteRes] : spritesToProcess) {
			m_spriteResources [std::move (libraryName)] [std::move (textureFile)] = PropGPUResourceManagerBackend::CreateSpriteResource (
				* spriteRes
			);
		}
	});

	resLoaderThread.join ();

	if (true == m_resourceSettings.dropCPUResources) {
		m_resourceManager.dropResources ();
	}
}

template <class PropGPUResourceManagerBackend>
void PropGPUResourceManager <PropGPUResourceManagerBackend>::loadMeshResources (const std::vector <std::pair <std::string, std::string>> & meshDescriptors) {
	std::vector <MeshResource> resources;
	resources.resize (meshDescriptors.size ());

	std::transform (
		std::execution::seq,
		meshDescriptors.cbegin (),
		meshDescriptors.cend (),
		resources.begin (),
		[this] (const std::pair <std::string, std::string> & descriptor) {
			// NOLINTNEXTLINE(hicpp-use-auto,modernize-use-auto)
			return PropGPUResourceManagerBackend::CreateMeshResource (
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

template <class PropGPUResourceManagerBackend>
void PropGPUResourceManager <PropGPUResourceManagerBackend>::loadTextureResources (const std::vector <std::pair <std::string, std::string>> & textureDescriptors) {
	std::vector <TextureResource> resources;
	resources.resize (textureDescriptors.size ());

	std::transform (
		std::execution::seq,
		textureDescriptors.cbegin (),
		textureDescriptors.cend (),
		resources.begin (),
		[this] (const std::pair <std::string, std::string> & descriptor) {
			return PropGPUResourceManagerBackend::CreateTextureResource (* m_resourceManager.propTextureResources ().at (descriptor.first).at (descriptor.second));
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

template <class PropGPUResourceManagerBackend>
const std::map <std::string, std::shared_ptr <PropLibrary>> & PropGPUResourceManager <PropGPUResourceManagerBackend>::propLibraries () const {
	return m_propLibraries;
}

template <class PropGPUResourceManagerBackend>
const PropGPUResourceManager <PropGPUResourceManagerBackend>::CPUResourceManager & PropGPUResourceManager <PropGPUResourceManagerBackend>::cpuResourceManager () {
	return m_resourceManager;
}

template <class PropGPUResourceManagerBackend>
const std::map <std::string, std::map <std::string, typename PropGPUResourceManager <PropGPUResourceManagerBackend>::MeshResource>> & PropGPUResourceManager <PropGPUResourceManagerBackend>::meshResources () const {
	return m_meshResources;

}
template <class PropGPUResourceManagerBackend>
const std::map <std::string, std::map <std::string, typename PropGPUResourceManager <PropGPUResourceManagerBackend>::TextureResource>> & PropGPUResourceManager <PropGPUResourceManagerBackend>::textureResources () const {
	return m_textureResources;
}

template <class PropGPUResourceManagerBackend>
const std::map <std::string, std::map <std::string, typename PropGPUResourceManager <PropGPUResourceManagerBackend>::SpriteResource>> & PropGPUResourceManager <PropGPUResourceManagerBackend>::spriteResources () const {
	return m_spriteResources;
}

}  // namespace MapMaster::Tanki

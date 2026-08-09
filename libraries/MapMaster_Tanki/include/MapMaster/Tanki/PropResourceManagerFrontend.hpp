# pragma once

# include <map>
# include <memory>
# include <string>
# include <utility>
# include <vector>

# include "MapMaster/Tanki/PropResourceManager.hpp"
# include "MapMaster/Tanki/PropResourceManagerRaylibAdapter.hpp"

namespace MapMaster::Tanki {

class Map;

// cppcheck-suppress-begin unusedStructMember
template <class PropResourceManagerAdapter>
class PropResourceManagerFrontend {
public:
	using MeshResource = PropResourceManagerAdapter::MeshResource;
	using TextureResource = PropResourceManagerAdapter::TextureResource;
	using SpriteInfo = PropResourceManagerAdapter::SpriteInfo;

public:
	explicit PropResourceManagerFrontend (bool parseCollisionPrimitives = false);

	void loadLibrary (const std::string & path);
	void loadMapLibraries (const Map & map, const std::string & libraryRootDir);
	void loadMapResources (const Map & map);
	void loadMapResources_OLD (const Map & map);

	/**
	 * @brief load and parse mesh files
	 *
	 * @param meshDescriptors {{libraryName, meshFileName}, ...}
	 */
	void loadMeshResources (const std::vector <std::pair <std::string, std::string>> & meshDescriptors);

	/**
	 * @brief load and parse texture files
	 *
	 * @param textureDescriptors {{libraryName, textureFile}, ...}
	 */
	void loadTextureResources (const std::vector <std::pair <std::string, std::string>> & textureDescriptors);

	[[nodiscard]] const PropResourceManager & resourceManager ();
	[[nodiscard]] const std::map <std::string, std::map <std::string, MeshResource>> & meshResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, TextureResource>> & textureResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, SpriteInfo>> & spriteInfos () const;

private:
	PropResourceManager m_resourceManager;
	std::string m_libraryRootDir;

	std::map <std::string, std::map <std::string, MeshResource>> m_meshResources;
	std::map <std::string, std::map <std::string, TextureResource>> m_textureResources;
	std::map <std::string, std::map <std::string, SpriteInfo>> m_spriteInfos;
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

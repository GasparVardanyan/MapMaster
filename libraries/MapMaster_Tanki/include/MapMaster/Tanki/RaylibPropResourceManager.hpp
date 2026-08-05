# pragma once

# include <raylib.h>
# include <map>
# include <memory>
# include <string>
# include <utility>
# include <vector>

# include "MapMaster/Tanki/PropResourceManager.hpp"

class Map;

// cppcheck-suppress-begin unusedStructMember
class RaylibPropResourceManager {
public:
	struct RaylibMeshResource {
		std::shared_ptr <Mesh> mesh;

		// TODO: clone()
	};

	struct RaylibTextureResource {
		Image image;
		std::shared_ptr <Texture2D> texture;

		// TODO: clone()
	};

	struct RaylibSpriteInfo {
		Vector2 origin;
		Vector2 size;
	};

public:
	explicit RaylibPropResourceManager (bool parseCollisionPrimitives = false);

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
	[[nodiscard]] const std::map <std::string, std::map <std::string, RaylibMeshResource>> & meshResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, RaylibTextureResource>> & textureResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, RaylibSpriteInfo>> & spriteInfos () const;

private:
	RaylibMeshResource loadMeshResource (PropResourceManager::PropMeshResource & meshResource);
	RaylibTextureResource loadTextureResource (const PropResourceManager::PropTextureResource & textureResource);
	static void unloadMeshResource (Mesh * mesh);
	static void unloadTextureResource (Texture2D * texture);

private:
	PropResourceManager m_resourceManager;
	std::string m_libraryRootDir;

	std::map <std::string, std::map <std::string, RaylibMeshResource>> m_meshResources;
	std::map <std::string, std::map <std::string, RaylibTextureResource>> m_textureResources;
	std::map <std::string, std::map <std::string, RaylibSpriteInfo>> m_spriteInfos;
};
// cppcheck-suppress-end unusedStructMember

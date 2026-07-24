# pragma once

# include <map>
# include <string>

# include <raylib.h>
# include <utility>
# include <vector>

# include "Map.hpp"
# include "PropResourceManager.hpp"

// cppcheck-suppress-begin unusedStructMember
class PropRaylibResourceManager {
public:
	struct RaylibMeshResource {
		Mesh mesh;
		Model model;

		// TODO: clone()
	};

	struct RaylibTextureResource {
		Image image;
		Texture2D texture;

		// TODO: clone()
	};

	struct RaylibSpriteInfo {
		Vector2 origin;
		Vector2 size;
	};

public:
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

private:
	PropResourceManager m_resourceManager;
	std::string m_libraryRootDir;

	std::map <std::string, std::map <std::string, RaylibMeshResource>> m_meshResources;
	std::map <std::string, std::map <std::string, RaylibTextureResource>> m_textureResources;
	std::map <std::string, std::map <std::string, RaylibSpriteInfo>> m_spriteInfos;
};
// cppcheck-suppress-end unusedStructMember

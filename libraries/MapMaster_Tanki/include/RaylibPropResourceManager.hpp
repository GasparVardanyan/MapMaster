# pragma once

# include <raylib.h>
# include <map>
# include <memory>
# include <string>
# include <utility>
# include <vector>

# include "PropResourceManager.hpp"

class Map;

// cppcheck-suppress-begin unusedStructMember
class RaylibPropResourceManager {
public:
	struct RaylibMeshResource {
		Mesh mesh;
		std::shared_ptr <Model> model;

		// TODO: clone()
	};

	struct RaylibMultiMeshResource {
		std::vector <Mesh> meshes;
		std::shared_ptr <Model> model;
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
	 * @brief load and parse mesh files
	 *
	 * @param meshDescriptors {{libraryName, meshFileName}, ...}
	 */
	void loadMultiMeshResources (const std::vector <std::pair <std::string, std::string>> & multiMeshDescriptors);

	/**
	 * @brief load and parse texture files
	 *
	 * @param textureDescriptors {{libraryName, textureFile}, ...}
	 */
	void loadTextureResources (const std::vector <std::pair <std::string, std::string>> & textureDescriptors);

	[[nodiscard]] const PropResourceManager & resourceManager ();
	[[nodiscard]] const std::map <std::string, std::map <std::string, RaylibMeshResource>> & meshResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, RaylibMultiMeshResource>> & multiMeshResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, RaylibTextureResource>> & textureResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, RaylibSpriteInfo>> & spriteInfos () const;

private:
	RaylibMeshResource loadMeshResource (PropResourceManager::PropMeshResource & meshResource);
	RaylibMultiMeshResource loadMultiMeshResource (PropResourceManager::PropMultiMeshResource & meshMultiResource);
	RaylibTextureResource loadTextureResource (const PropResourceManager::PropTextureResource & textureResource);
	static void unloadMeshResource (Model * model);
	static void unloadTextureResource (Texture2D * texture);

private:
	PropResourceManager m_resourceManager;
	std::string m_libraryRootDir;

	std::map <std::string, std::map <std::string, RaylibMeshResource>> m_meshResources;
	std::map <std::string, std::map <std::string, RaylibMultiMeshResource>> m_multiMeshResources;
	std::map <std::string, std::map <std::string, RaylibTextureResource>> m_textureResources;
	std::map <std::string, std::map <std::string, RaylibSpriteInfo>> m_spriteInfos;
};
// cppcheck-suppress-end unusedStructMember

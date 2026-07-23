# pragma once

# include <map>
# include <string>

# include <raylib.h>

# include "Map.hpp"
# include "PropResourceManager.hpp"

class PropRaylibResourceManager {
public:
	struct RaylibMeshResource {
		Mesh mesh;
		Model model;
	};

	struct RaylibTextureResource {
		Texture2D texture = {};
	};

	struct RaylibSpriteInfo {
		Vector2 origin;
		Vector2 size;
	};

public:
	PropResourceManager & resourceManager ();
	void loadLibrary (const std::string & path);
	void loadMapLibraries (const Map & map, const std::string & libraryRootDir);
	void loadMapResources (const Map & map);

	[[nodiscard]] const std::map <std::string, std::map <std::string, RaylibMeshResource>> & meshResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, RaylibTextureResource>> & textureResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, RaylibSpriteInfo>> & spriteInfos () const;

private:
	PropResourceManager m_resourceManager;
	std::string m_libraryRootDir;

	std::map <std::string, std::map <std::string, RaylibMeshResource>> m_meshResources;
	std::map <std::string, std::map <std::string, RaylibTextureResource>> m_textureResources;
	std::map <std::string, std::map <std::string, RaylibSpriteInfo>> m_spriteInfos;

};

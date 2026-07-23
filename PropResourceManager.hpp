# pragma once

# include <functional>
# include <map>
# include <memory>
# include <string>
# include <type_traits>
# include <utility>
# include <vector>

# include "Map.hpp"
# include "PropLibrary.hpp"

// cppcheck-suppress-begin unusedStructMember
class PropResourceManager {
public:
	using ResourceLoadCallback = std::function <void (std::string, std::string)>;
	using MapResourcesLoadCallback = std::function <void ()>;

	struct PropMeshResource {
		using VertexType = float;
		static_assert (std::is_floating_point_v <VertexType>);
		using NormalType = float;
		static_assert (std::is_floating_point_v <NormalType>);
		using TexCoordType = float;
		static_assert (std::is_floating_point_v <TexCoordType>);
		using IndexType = unsigned short; // NOLINT(google-runtime-int)
		static_assert (std::is_integral_v <IndexType>);

		std::string textureFile;
		std::vector <VertexType> vertexBuffer;
		std::vector <NormalType> normalBuffer;
		std::vector <TexCoordType> uvBuffer;
		std::vector <IndexType> indexBuffer;
	};

	struct PropTextureResource {
		std::shared_ptr <unsigned char> pixBuffer;
		int width = -1;
		int height = -1;
		int channels = -1;
	};

public:
	void addPropLibrary (const PropLibrary & propLibrary);
	void addPropLibrary (PropLibrary && propLibrary);

	void loadMapResources (const Map & map);

	/**
	 * @brief load and parse mesh files
	 *
	 * @param meshDescriptors {{libraryName, meshFileName}, ...}
	 */
	void loadMeshResources (const std::vector <std::pair <std::string, std::string>> & meshDescriptors);

	/**
	 * @brief load and parse texture files
	 *
	 * @param textureDescriptors {{libraryName, {diffuseFileName, opacityFileName}}}
	 */
	void loadTextureResources (const std::vector <std::pair <std::string, std::pair <std::string, std::string>>> & textureDescriptors);

	[[nodiscard]] const PropMeshResource & getMeshResource (const std::string & libraryName, const std::string & groupName, const std::string & propName) const;
	[[nodiscard]] const PropTextureResource & getTextureResource (const std::string & libraryName, const std::string & groupName, const std::string & propMeshName, const std::string & textureName) const;
	[[nodiscard]] const PropTextureResource & getTextureResource (const std::string & libraryName, const std::string & groupName, const std::string & propSpriteName) const;
	[[nodiscard]] const std::map <std::string, PropLibrary> & propLibraries () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, PropMeshResource>> & propMeshResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, PropTextureResource>> & propTextureResources () const;

private:
	PropMeshResource loadMeshResources (const std::string & libraryName, const std::string & meshFile);
	PropTextureResource loadTextureResources (const std::string & libraryName, const std::string & diffuseFile, const std::string & opacityFile);

private:
	std::map <std::string, PropLibrary> m_propLibraries;
	std::map <std::string, std::map <std::string, PropMeshResource>> m_propMeshResources;
	std::map <std::string, std::map <std::string, PropTextureResource>> m_propTextureResources;

	ResourceLoadCallback m_meshResourceLoadCallback = nullptr;
	ResourceLoadCallback m_textureResourceLoadCallback = nullptr;
	MapResourcesLoadCallback m_mapResourceLoadCallback = nullptr;
};
// cppcheck-suppress-end unusedStructMember

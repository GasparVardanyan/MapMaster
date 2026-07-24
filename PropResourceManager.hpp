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

// FIXME: CRITICAL!
// free gpu and ram resources in dtor
// implement interfaces to free up ram and vram resources
// TODO: implement an interface to load/unload from/to ram/vram

// cppcheck-suppress-begin unusedStructMember
class PropResourceManager {
public:
	using ResourceLoadCallback = std::function <void (std::string, std::string)>;
	using MapResourcesLoadCallback = std::function <void ()>;

	struct PropMeshResource {
		using VertexType = float;
		using NormalType = float;
		using TexCoordType = float;
		using IndexType = unsigned short; // NOLINT(google-runtime-int)

		static_assert (std::is_floating_point_v <VertexType>);
		static_assert (std::is_floating_point_v <NormalType>);
		static_assert (std::is_floating_point_v <TexCoordType>);
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

		// TODO: replace with big five
		PropTextureResource clone ();
	};

	enum class OverlapBehaviour : unsigned char {
		Ignore, Override
	};

public:
	void addPropLibrary (std::shared_ptr <PropLibrary> propLibrary);
	void dropResources ();
	void removePropLibrary (const std::string & name);
	void clearPropLibraries ();
	void setOverlapBehaviour (OverlapBehaviour overlapBehaviour);

	/**
	 * @brief load and parse mesh files
	 *
	 * @param meshDescriptors {{libraryName, meshFileName}, ...}
	 */
	void loadMeshResources (const std::vector <std::pair <std::string, std::string>> & meshDescriptors);

	/**
	 * @brief load and parse texture files
	 *
	 * @param textureDescriptors {{libraryName, {diffuseFileName, alphaFileName}}}
	 */
	void loadTextureResources (const std::vector <std::pair <std::string, std::pair <std::string, std::string>>> & textureDescriptors);

	void loadMapResources (const Map & map);

	[[nodiscard]] const std::map <std::string, std::shared_ptr <PropLibrary>> & propLibraries () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, PropMeshResource>> & propMeshResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, PropTextureResource>> & propTextureResources () const;

	[[nodiscard]] const PropMeshResource & getMeshResource (const std::string & libraryName, const std::string & groupName, const std::string & propName) const;
	[[nodiscard]] const PropTextureResource & getTextureResource (const std::string & libraryName, const std::string & groupName, const std::string & propMeshName, const std::string & textureName) const;
	[[nodiscard]] const PropTextureResource & getTextureResource (const std::string & libraryName, const std::string & groupName, const std::string & propSpriteName) const;

	void setMeshResourceLoadCallback (const ResourceLoadCallback & callback);
	void setTextureResourceLoadCallback (const ResourceLoadCallback & callback);
	void setMapMeshResourcesLoadCallback (const MapResourcesLoadCallback & callback);
	void setMapTextureResourcesLoadCallback (const MapResourcesLoadCallback & callback);
	void clearCallbacks ();

private:
	PropMeshResource loadMeshResource (const std::string & libraryName, const std::string & meshFile);
	PropTextureResource loadTextureResource (const std::string & libraryName, const std::string & diffuseFile, const std::string & alphaFile);

private:
	std::map <std::string, std::shared_ptr <PropLibrary>> m_propLibraries;
	std::map <std::string, std::map <std::string, PropMeshResource>> m_propMeshResources;
	std::map <std::string, std::map <std::string, PropTextureResource>> m_propTextureResources;

	struct {
		ResourceLoadCallback meshResourceLoad = nullptr;
		ResourceLoadCallback textureResourceLoad = nullptr;
		MapResourcesLoadCallback mapMeshResourcesLoad = nullptr;
		MapResourcesLoadCallback mapTextureResourcesLoad = nullptr;
	} m_callbacks;

	OverlapBehaviour m_overlapBehaviour = OverlapBehaviour::Ignore;
};
// cppcheck-suppress-end unusedStructMember

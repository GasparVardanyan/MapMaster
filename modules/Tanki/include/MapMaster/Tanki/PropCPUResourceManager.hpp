# pragma once

# include <functional>
# include <map>
# include <memory>
# include <string>
# include <tuple>
# include <type_traits>
# include <utility>
# include <vector>

# include "MapMaster/Tanki/PropCPUResourceManagerRaylibAdapter.hpp"

namespace MapMaster::Tanki {

class Map;
class PropLibrary;

// cppcheck-suppress-begin unusedStructMember
template <class PropCPUResourceManagerAdapter = PropCPUResourceManagerRaylibAdapter>
class PropCPUResourceManager {
public:
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

	struct Collider {
		using VertexType = float;

		static_assert (std::is_floating_point_v <VertexType>);

		struct BoxCollider {
			struct {
				VertexType x, y, z;
			} vMin, vMax;
		};
		struct RectCollider {
			struct {
				VertexType x, y, z;
			} v1, v2, v3, v4;
		};
		struct TriangleCollider {
			struct {
				VertexType x, y, z;
			} v1, v2, v3;
		};

		std::vector <BoxCollider> boxColliders;
		std::vector <RectCollider> rectColliders;
		std::vector <TriangleCollider> triangleColliders;
	};

	using MeshResourceLoadCallback = std::function <void (std::string, std::string, std::shared_ptr <PropMeshResource>, std::shared_ptr <Collider>)>;
	using TextureResourceLoadCallback = std::function <void (std::string, std::string, std::shared_ptr <PropTextureResource>)>;
	using MapResourcesLoadCallback = std::function <void ()>;

private:
	struct ParsedMeshInfo {
		std::vector <PropMeshResource> meshResources;
		Collider collider;
	};

public:
	explicit PropCPUResourceManager (bool parseCollisionPrimitives = false);

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
	void loadTextureResources (const std::vector <std::tuple <std::string, std::string, std::string>> & textureDescriptors);

	void loadMapResources (const Map & map);

	[[nodiscard]] const std::map <std::string, std::shared_ptr <PropLibrary>> & propLibraries () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, std::shared_ptr <PropMeshResource>>> & propMeshResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, std::shared_ptr <PropTextureResource>>> & propTextureResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, std::shared_ptr <Collider>>> & colliders () const;

	[[nodiscard]] const PropMeshResource & getMeshResource (const std::string & libraryName, const std::string & groupName, const std::string & propName) const;
	[[nodiscard]] const PropTextureResource & getTextureResource (const std::string & libraryName, const std::string & groupName, const std::string & propMeshName, const std::string & textureName) const;
	[[nodiscard]] const PropTextureResource & getTextureResource (const std::string & libraryName, const std::string & groupName, const std::string & propSpriteName) const;

	void setMeshResourceLoadCallback (const MeshResourceLoadCallback & callback);
	void setTextureResourceLoadCallback (const TextureResourceLoadCallback & callback);
	void setMapMeshResourcesLoadCallback (const MapResourcesLoadCallback & callback);
	void setMapTextureResourcesLoadCallback (const MapResourcesLoadCallback & callback);
	void clearCallbacks ();

private:
	ParsedMeshInfo loadMeshResource (const std::string & libraryName, const std::string & meshFile);
	PropTextureResource loadTextureResource (const std::string & libraryName, const std::string & diffuseFile, const std::string & alphaFile);

private:
	std::map <std::string, std::shared_ptr <PropLibrary>> m_propLibraries;
	std::map <std::string, std::map <std::string, std::shared_ptr <PropMeshResource>>> m_propMeshResources;
	std::map <std::string, std::map <std::string, std::shared_ptr <PropTextureResource>>> m_propTextureResources;
	std::map <std::string, std::map <std::string, std::shared_ptr <Collider>>> m_colliders;

	struct {
		MeshResourceLoadCallback meshResourceLoad = nullptr;
		TextureResourceLoadCallback textureResourceLoad = nullptr;
		MapResourcesLoadCallback mapMeshResourcesLoad = nullptr;
		MapResourcesLoadCallback mapTextureResourcesLoad = nullptr;
	} m_callbacks;

	OverlapBehaviour m_overlapBehaviour = OverlapBehaviour::Ignore;
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
	const bool m_parseCollisionPrimitives;
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

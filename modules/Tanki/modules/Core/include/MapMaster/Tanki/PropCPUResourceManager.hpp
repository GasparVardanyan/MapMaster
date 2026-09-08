# pragma once

# include <cstdio>
# include <functional>
# include <map>
# include <memory>
# include <optional>
# include <string>
# include <tuple>
# include <type_traits>
# include <utility>
# include <vector>

# include "MapMaster/Tanki/PropMetaData.hpp"
# include "MapMaster/Tanki/Utils/ParallelTask.hpp"



extern "C" { struct aiScene; }

namespace MapMaster::Tanki {

namespace PropMetaData { struct Mesh; struct Sprite; struct Texture; }

class Map;
class PropLibrary;

template <class PropCPUResourceManagerBackend, typename = void>
struct IsPropCPUResourceManagerBackend : std::false_type {};

template <class PropCPUResourceManagerBackend>
struct IsPropCPUResourceManagerBackend <
	PropCPUResourceManagerBackend,
	std::void_t <
		std::enable_if_t <std::is_same_v <
			decltype (& PropCPUResourceManagerBackend::AssimpImporterRemoveComponentFlags),
			int *
		>>,
		std::enable_if_t <std::is_same_v <
			decltype (& PropCPUResourceManagerBackend::AssimpPostProcessorSteps),
			unsigned int *
		>>,

		typename PropCPUResourceManagerBackend::PropMeshResource,
		typename PropCPUResourceManagerBackend::PropTextureResource,

		decltype (PropCPUResourceManagerBackend::PropMeshResource::meta),
		decltype (PropCPUResourceManagerBackend::PropTextureResource::meta),

		std::enable_if_t <std::is_same_v <
			decltype (PropCPUResourceManagerBackend::PropMeshResource::meta),
			std::shared_ptr <PropMetaData::Mesh>
		>>,

		std::enable_if_t <std::is_same_v <
			decltype (PropCPUResourceManagerBackend::PropTextureResource::meta),
			std::shared_ptr <PropMetaData::Texture>
		>>,

		std::enable_if_t <std::is_invocable_v <
			decltype (PropCPUResourceManagerBackend::ParseMeshResource),
			const aiScene *
		>>,
		std::enable_if_t <std::is_invocable_v <
			decltype (PropCPUResourceManagerBackend::ParseTextureResource),
			std::FILE *, std::FILE *
		>>,

		std::enable_if_t <std::is_same_v <
			typename PropCPUResourceManagerBackend::PropMeshResource,
			std::invoke_result_t <
				decltype (PropCPUResourceManagerBackend::ParseMeshResource),
				const aiScene *
			>
		>>,
		std::enable_if_t <std::is_same_v <
			typename PropCPUResourceManagerBackend::PropTextureResource,
			std::invoke_result_t <
				decltype (PropCPUResourceManagerBackend::ParseTextureResource),
				std::FILE *, std::FILE *
			>
		>>
	>
> : std::true_type {};

// cppcheck-suppress-begin unusedStructMember
template <class PropCPUResourceManagerBackend>
class PropCPUResourceManager {
public:
	using Backend = std::enable_if_t <
		IsPropCPUResourceManagerBackend <PropCPUResourceManagerBackend>::value,
		PropCPUResourceManagerBackend
	>;
	using PropMeshResource = Backend::PropMeshResource;
	using PropTextureResource = Backend::PropTextureResource;

	using MeshLoaderTask = Utils::ParallelTask <PropCPUResourceManager, PropMeshResource, std::string, std::string>;
	using TextureLoaderTask = Utils::ParallelTask <PropCPUResourceManager, PropTextureResource, std::string, std::string, std::string>;

	enum class OverlapBehaviour : unsigned char {
		Ignore, Override
	};

public:
	explicit PropCPUResourceManager (bool parseCollisionPrimitives = false, bool collectCpuData = true);

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
	void loadMeshResources (
		const std::vector <std::tuple <std::string, std::string>> & meshDescriptors,
		std::optional <std::reference_wrapper <std::vector <std::shared_ptr <PropMetaData::Mesh>>>> meta = {}
	);

	/**
	 * @brief load and parse texture files
	 *
	 * @param textureDescriptors {{libraryName, diffuseFileName, alphaFileName}, ...}
	 */
	void loadTextureResources (
		const std::vector <std::tuple <std::string, std::string, std::string>> & textureDescriptors,
		std::optional <std::reference_wrapper <std::vector <std::shared_ptr <PropMetaData::Texture>>>> meta = {}
	);

	void loadMapResources (const Map & map);
	void loadPropLibraryResources (const PropLibrary & propLibrary);

	[[nodiscard]] const std::map <std::string, std::shared_ptr <PropLibrary>> & propLibraries () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, std::shared_ptr <PropMeshResource>>> & propMeshResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, std::shared_ptr <PropTextureResource>>> & propTextureResources () const;

	[[nodiscard]] const PropMeshResource & getMeshResource (const std::string & libraryName, const std::string & groupName, const std::string & propName) const;
	[[nodiscard]] const PropTextureResource & getTextureResource (const std::string & libraryName, const std::string & groupName, const std::string & propMeshName, const std::string & textureName) const;
	[[nodiscard]] const PropTextureResource & getTextureResource (const std::string & libraryName, const std::string & groupName, const std::string & propSpriteName) const;

	MeshLoaderTask & meshLoader ();
	TextureLoaderTask & textureLoader ();

private:
	PropMeshResource loadMeshResource (const std::string & libraryName, const std::string & meshFile) const;
	PropTextureResource loadTextureResource (const std::string & libraryName, const std::string & diffuseFile, const std::string & alphaFile) const;

private:
	std::map <std::string, std::shared_ptr <PropLibrary>> m_propLibraries;
	std::map <std::string, std::map <std::string, std::shared_ptr <PropMeshResource>>> m_propMeshResources;
	std::map <std::string, std::map <std::string, std::shared_ptr <PropTextureResource>>> m_propTextureResources;
	// std::map <std::string, std::map <std::string, std::shared_ptr <PropMetaData::Sprite>>> m_propSpriteMetaDatas;

	MeshLoaderTask m_meshLoader;
	TextureLoaderTask m_textureLoader;

	OverlapBehaviour m_overlapBehaviour = OverlapBehaviour::Ignore;
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
	const bool m_parseCollisionPrimitives;
	const bool m_collectCpuData;
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

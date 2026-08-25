# pragma once

# include <map>
# include <memory>
# include <string>
# include <type_traits>
# include <utility>
# include <vector>

# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/PropLibrary.hpp"

namespace MapMaster::Tanki {

class Map;
template <class PropCPUResourceManagerBackend> class PropCPUResourceManager;

template <class PropGPUResourceManagerBackend, typename = void>
struct IsPropGPUResourceManagerBackend : std::false_type {};

template <class PropGPUResourceManagerBackend>
struct IsPropGPUResourceManagerBackend <
	PropGPUResourceManagerBackend,
	std::void_t <
		typename PropGPUResourceManagerBackend::CPUResourceManagerBackend,
		typename PropGPUResourceManagerBackend::CPUResourceManager,

		std::enable_if_t <std::is_same_v <
			typename PropGPUResourceManagerBackend::CPUResourceManager,
			PropCPUResourceManager <typename PropGPUResourceManagerBackend::CPUResourceManagerBackend>
		>>,

		typename PropGPUResourceManagerBackend::MeshResource,
		typename PropGPUResourceManagerBackend::TextureResource,
		typename PropGPUResourceManagerBackend::SpriteInfo,

		std::enable_if_t <std::is_invocable_v <
			decltype (PropGPUResourceManagerBackend::template CreateMeshResource <true>),
			typename PropGPUResourceManagerBackend::CPUResourceManager::PropMeshResource &
		>>,
		std::enable_if_t <std::is_invocable_v <
			decltype (PropGPUResourceManagerBackend::template CreateTextureResource <true>),
			const typename PropGPUResourceManagerBackend::CPUResourceManager::PropTextureResource &
		>>,
		std::enable_if_t <std::is_invocable_v <
			decltype (PropGPUResourceManagerBackend::CreateSpriteInfo),
			const PropLibrary::PropSprite &,
			const typename PropGPUResourceManagerBackend::CPUResourceManager::PropTextureResource &
		>>,

		std::enable_if_t <std::is_same_v <
			typename PropGPUResourceManagerBackend::MeshResource,
			std::invoke_result_t <
				decltype (PropGPUResourceManagerBackend::template CreateMeshResource <true>),
				typename PropGPUResourceManagerBackend::CPUResourceManager::PropMeshResource &
			>
		>>,
		std::enable_if_t <std::is_same_v <
			typename PropGPUResourceManagerBackend::TextureResource,
			std::invoke_result_t <
				decltype (PropGPUResourceManagerBackend::template CreateTextureResource <true>),
				const typename PropGPUResourceManagerBackend::CPUResourceManager::PropTextureResource &
			>
		>>,
		std::enable_if_t <std::is_same_v <
			typename PropGPUResourceManagerBackend::SpriteInfo,
			std::invoke_result_t <
				decltype (PropGPUResourceManagerBackend::CreateSpriteInfo),
				const PropLibrary::PropSprite &,
				const typename PropGPUResourceManagerBackend::CPUResourceManager::PropTextureResource &
			>
		>>,

		std::enable_if_t <std::is_invocable_v <
			decltype (PropGPUResourceManagerBackend::UploadMeshResource),
			typename PropGPUResourceManagerBackend::MeshResource &
		>>,
		std::enable_if_t <std::is_invocable_v <
			decltype (PropGPUResourceManagerBackend::UploadTextureResource),
			typename PropGPUResourceManagerBackend::TextureResource &
		>>,

		void
	>
> : std::true_type {};

// cppcheck-suppress-begin unusedStructMember
template <class PropGPUResourceManagerBackend>
class PropGPUResourceManager {
public:
	using Backend = std::enable_if_t <
		IsPropGPUResourceManagerBackend <PropGPUResourceManagerBackend>::value,
		PropGPUResourceManagerBackend
	>;
	using CPUResourceManager = Backend::CPUResourceManager;
	using MeshResource = Backend::MeshResource;
	using TextureResource = Backend::TextureResource;
	using SpriteInfo = Backend::SpriteInfo;

public:
	explicit PropGPUResourceManager (bool parseCollisionPrimitives = false);

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

	[[nodiscard]] const CPUResourceManager & cpuResourceManager ();
	[[nodiscard]] const std::map <std::string, std::map <std::string, MeshResource>> & meshResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, TextureResource>> & textureResources () const;
	[[nodiscard]] const std::map <std::string, std::map <std::string, SpriteInfo>> & spriteInfos () const;

private:
	CPUResourceManager m_resourceManager;
	std::string m_libraryRootDir;

	std::map <std::string, std::map <std::string, MeshResource>> m_meshResources;
	std::map <std::string, std::map <std::string, TextureResource>> m_textureResources;
	std::map <std::string, std::map <std::string, SpriteInfo>> m_spriteInfos;
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

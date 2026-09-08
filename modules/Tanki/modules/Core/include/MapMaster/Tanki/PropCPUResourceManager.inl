# include "MapMaster/Tanki/PropCPUResourceManager.hpp"

# include <algorithm>
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <execution>
# include <initializer_list>
# include <map>
# include <memory>
# include <set>
# include <stdexcept>
# include <string>
# include <tuple>
# include <utility>
# include <vector>

# include <assimp/Importer.hpp>
# include <assimp/config.h>
# include <assimp/material.h>
# include <assimp/matrix4x4.h>
# include <assimp/mesh.h>
# include <assimp/postprocess.h>
# include <assimp/scene.h>
# include <assimp/types.h>
# include <assimp/vector3.h>

# include "MapMaster/Tanki/Map.hpp"
# include "MapMaster/Tanki/PropLibrary.hpp"
# include "MapMaster/Tanki/PropMetaData.hpp"

using namespace MapMaster::Tanki;



template <class PropCPUResourceManagerBackend>
PropCPUResourceManager <PropCPUResourceManagerBackend>::PropCPUResourceManager (bool parseCollisionPrimitives, bool collectCpuData)
	: m_parseCollisionPrimitives (parseCollisionPrimitives)
	, m_collectCpuData (collectCpuData)
	, m_meshLoader (* this, & PropCPUResourceManager::loadMeshResource)
	, m_textureLoader (* this, & PropCPUResourceManager::loadTextureResource)
{
}

template <class PropCPUResourceManagerBackend>
// NOLINTNEXTLINE(performance-unnecessary-value-param)
void PropCPUResourceManager <PropCPUResourceManagerBackend>::addPropLibrary (std::shared_ptr <PropLibrary> propLibrary) {
	const std::string & libraryName = propLibrary->name ();
	m_propLibraries.try_emplace (
		libraryName,
		std::move (propLibrary)
	);
}

template <class PropCPUResourceManagerBackend>
void PropCPUResourceManager <PropCPUResourceManagerBackend>::dropResources () {
	m_propMeshResources.clear ();
	m_propTextureResources.clear ();
}

template <class PropCPUResourceManagerBackend>
void PropCPUResourceManager <PropCPUResourceManagerBackend>::removePropLibrary (const std::string & name) {
	m_propLibraries.erase (name);
}

template <class PropCPUResourceManagerBackend>
void PropCPUResourceManager <PropCPUResourceManagerBackend>::clearPropLibraries () {
	m_propLibraries = {};
}

template <class PropCPUResourceManagerBackend>
void PropCPUResourceManager <PropCPUResourceManagerBackend>::setOverlapBehaviour (OverlapBehaviour overlapBehaviour) {
	throw std::runtime_error ("PropCPUResourceManager::OverlapBehaviour is not in effect");
	m_overlapBehaviour = overlapBehaviour; // cppcheck-suppress unreachableCode
}




//  _      ____          _____  ______ _____   _____
// | |    / __ \   /\   |  __ \|  ____|  __ \ / ____|
// | |   | |  | | /  \  | |  | | |__  | |__) | (___
// | |   | |  | |/ /\ \ | |  | |  __| |  _  / \___ \
// | |___| |__| / ____ \| |__| | |____| | \ \ ____) |
// |______\____/_/    \_\_____/|______|_|  \_\_____/
//

template <class PropCPUResourceManagerBackend>
// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
void PropCPUResourceManager <PropCPUResourceManagerBackend>::loadMeshResources (std::vector <std::tuple <std::string, std::string>> && meshDescriptors) {
	if (true == m_collectCpuData) {
		std::vector <std::shared_ptr <PropMeshResource>> resources = m_meshLoader.template run <true, true> (meshDescriptors);

		std::size_t mI = 0;

		for (auto & [libraryName, meshFile] : meshDescriptors) {
			m_propMeshResources [std::move (libraryName)] [std::move (meshFile)] = std::move (resources [mI]);

			mI++;
		}
	}
	else {
		m_meshLoader.template run <false, true> (meshDescriptors);
	}
}

template <class PropCPUResourceManagerBackend>
// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
void PropCPUResourceManager <PropCPUResourceManagerBackend>::loadTextureResources (std::vector <std::tuple <std::string, std::string, std::string>> && textureDescriptors) {
	if (true == m_collectCpuData) {
		std::vector <std::shared_ptr <PropTextureResource>> resources = m_textureLoader.template run <true, true> (textureDescriptors);

		std::size_t tI = 0;
		for (auto & [libraryName, diffuseFile, _] : textureDescriptors) {
			m_propTextureResources [std::move (libraryName)] [std::move (diffuseFile)] = (std::move (resources [tI]));

			tI++;
		}
	}
	else {
		m_textureLoader.template run <false, true> (textureDescriptors);
	}
}

template <class PropCPUResourceManagerBackend>
void PropCPUResourceManager <PropCPUResourceManagerBackend>::loadMapResources (const Map & map) {
	std::vector <std::tuple <std::string, std::string>> meshDescriptors;
	std::vector <std::tuple <std::string, std::string, std::string>> textureDescriptors;

	std::map <std::string, std::map <std::string, std::set <std::string>>> defaultTextures;

	for (const auto & [libraryName, groupNames] : map.mapObjects ()) {
		const PropLibrary & library = * m_propLibraries.at (libraryName);
		const std::map <std::string, PropLibrary::Group> & groups = library.groups ();

		for (const auto & [groupName, propNames] : groupNames) {
			const PropLibrary::Group & group = groups.at (groupName);
			const std::map <std::string, PropLibrary::PropMesh> & groupMeshes = group.meshes;
			const std::map <std::string, PropLibrary::PropSprite> & groupSprites = group.sprites;

			for (const auto & [propName, propList] : propNames) {
				if (auto mIt = groupMeshes.find (propName); groupMeshes.end () != mIt) {
					meshDescriptors.emplace_back (
						libraryName,
						mIt->second.file
					);

					for (const Map::MapObject & propObject : propList) {
						if (false == propObject.textureName.empty ()) {
							std::string diffuseFile = mIt->second.textures.at (propObject.textureName);
							std::string alphaFile;

							if (auto it = library.alphaMap ().find (diffuseFile); it != library.alphaMap ().end ()) {
								alphaFile = it->second;
							}
							if (auto it = library.diffuseMap ().find (diffuseFile); it != library.diffuseMap ().end ()) {
								diffuseFile = it->second;
							}
							textureDescriptors.emplace_back (libraryName, diffuseFile, alphaFile);
						}
						else {
							defaultTextures [libraryName] [groupName].insert (propName);
						}
					}
				}
				else if (auto sIt = groupSprites.find (propName); groupSprites.end () != sIt) {
					const PropLibrary::PropSprite & sprite = sIt->second;
					std::string diffuseFile = sprite.diffuseFile;
					std::string alphaFile;

					if (auto it = library.alphaMap ().find (diffuseFile); it != library.alphaMap ().end ()) {
						alphaFile = it->second;
					}
					if (auto it = library.diffuseMap ().find (diffuseFile); it != library.diffuseMap ().end ()) {
						diffuseFile = it->second;
					}

					textureDescriptors.emplace_back (libraryName, diffuseFile, alphaFile);
				}
			}
		}
	}

	std::sort (std::execution::par_unseq, meshDescriptors.begin (), meshDescriptors.end ());
	meshDescriptors.erase (std::unique (std::execution::par_unseq, meshDescriptors.begin (), meshDescriptors.end ()), meshDescriptors.end ());

	loadMeshResources (std::move (meshDescriptors));
	meshDescriptors.clear ();

	for (const auto & [libraryName, groupData] : defaultTextures) {
		const PropLibrary & library = * m_propLibraries.at (libraryName);
		const std::map <std::string, PropLibrary::Group> & groups = library.groups ();
		const std::map <std::string, std::shared_ptr <PropMeshResource>> & libraryMeshResources = m_propMeshResources.at (libraryName);

		for (const auto & [groupName, props] : groupData) {
			const PropLibrary::Group & group = groups.at (groupName);

			for (const std::string & propName : props) {
				const PropLibrary::PropMesh & prop = group.meshes.at (propName);

				std::string diffuseFile = libraryMeshResources.at (prop.file)->meta->textureFile;
				std::string alphaFile;

				if (auto it = library.alphaMap ().find (diffuseFile); it != library.alphaMap ().end ()) {
					alphaFile = it->second;
				}
				if (auto it = library.diffuseMap ().find (diffuseFile); it != library.diffuseMap ().end ()) {
					diffuseFile = it->second;
				}
				textureDescriptors.emplace_back (libraryName, diffuseFile, alphaFile);
			}
		}
	}

	std::sort (std::execution::par_unseq, textureDescriptors.begin (), textureDescriptors.end ());
	textureDescriptors.erase (std::unique (std::execution::par_unseq, textureDescriptors.begin (), textureDescriptors.end ()), textureDescriptors.end ());

	loadTextureResources (std::move (textureDescriptors));
	textureDescriptors.clear ();
}

template <class PropCPUResourceManagerBackend>
void PropCPUResourceManager <PropCPUResourceManagerBackend>::loadPropLibraryResources (const PropLibrary & propLibrary) {
}



//  _      ____          _____  ______ _____    _    _ ______ _      _____  ______ _____   _____
// | |    / __ \   /\   |  __ \|  ____|  __ \  | |  | |  ____| |    |  __ \|  ____|  __ \ / ____|
// | |   | |  | | /  \  | |  | | |__  | |__) | | |__| | |__  | |    | |__) | |__  | |__) | (___
// | |   | |  | |/ /\ \ | |  | |  __| |  _  /  |  __  |  __| | |    |  ___/|  __| |  _  / \___ \
// | |___| |__| / ____ \| |__| | |____| | \ \  | |  | | |____| |____| |    | |____| | \ \ ____) |
// |______\____/_/    \_\_____/|______|_|  \_\ |_|  |_|______|______|_|    |______|_|  \_\_____/
//

template <class PropCPUResourceManagerBackend>
PropCPUResourceManager <PropCPUResourceManagerBackend>::PropMeshResource PropCPUResourceManager <PropCPUResourceManagerBackend>::loadMeshResource (const std::string & libraryName, const std::string & meshFile) const {
	const std::string meshPath = m_propLibraries.at (libraryName)->path () + "/" + meshFile;

	Assimp::Importer importer;
	importer.SetPropertyInteger (
		AI_CONFIG_PP_RVC_FLAGS,
		PropCPUResourceManagerBackend::AssimpImporterRemoveComponentFlags
	);

	const aiScene * scene = importer.ReadFile (meshPath, PropCPUResourceManagerBackend::AssimpPostProcessorSteps);

	return PropCPUResourceManagerBackend::ParseMeshResource (scene);
}

template <class PropCPUResourceManagerBackend>
PropCPUResourceManager <PropCPUResourceManagerBackend>::PropTextureResource PropCPUResourceManager <PropCPUResourceManagerBackend>::loadTextureResource (const std::string & libraryName, const std::string & diffuseFile, const std::string & alphaFile) const {
	const std::string diffusePath = m_propLibraries.at (libraryName)->path () + "/" + diffuseFile;

	std::FILE * diffuseFileHandle = std::fopen (diffusePath.c_str (), "rb");
	std::FILE * alphaFileHandle = nullptr; {
		if (false == alphaFile.empty ()) {
			const std::string alphaPath = m_propLibraries.at (libraryName)->path () + "/" + alphaFile;
			alphaFileHandle = std::fopen (alphaPath.c_str (), "rb");
		}
	}

	PropTextureResource resource = PropCPUResourceManagerBackend::ParseTextureResource (diffuseFileHandle, alphaFileHandle);


	// NOLINTNEXTLINE(cert-err33-c,cppcoreguidelines-owning-memory)
	std::fclose (diffuseFileHandle);
	if (nullptr != alphaFileHandle) {
		// NOLINTNEXTLINE(cert-err33-c,cppcoreguidelines-owning-memory)
		std::fclose (alphaFileHandle);
	}

	return resource;
}



//   _____ ______ _______ _______ ______ _____   _____
//  / ____|  ____|__   __|__   __|  ____|  __ \ / ____|
// | |  __| |__     | |     | |  | |__  | |__) | (___
// | | |_ |  __|    | |     | |  |  __| |  _  / \___ \
// | |__| | |____   | |     | |  | |____| | \ \ ____) |
//  \_____|______|  |_|     |_|  |______|_|  \_\_____/
//

template <class PropCPUResourceManagerBackend>
const std::map <std::string, std::shared_ptr <PropLibrary>> & PropCPUResourceManager <PropCPUResourceManagerBackend>::propLibraries () const {
	return m_propLibraries;
}

template <class PropCPUResourceManagerBackend>
const std::map <std::string, std::map <std::string, std::shared_ptr <typename PropCPUResourceManager <PropCPUResourceManagerBackend>::PropMeshResource>>> & PropCPUResourceManager <PropCPUResourceManagerBackend>::propMeshResources () const {
	return m_propMeshResources;
}

template <class PropCPUResourceManagerBackend>
const std::map <std::string, std::map <std::string, std::shared_ptr <typename PropCPUResourceManager <PropCPUResourceManagerBackend>::PropTextureResource>>> & PropCPUResourceManager <PropCPUResourceManagerBackend>::propTextureResources () const {
	return m_propTextureResources;
}

template <class PropCPUResourceManagerBackend>
const PropCPUResourceManager <PropCPUResourceManagerBackend>::PropMeshResource & PropCPUResourceManager <PropCPUResourceManagerBackend>::getMeshResource (const std::string & libraryName, const std::string & groupName, const std::string & propName) const {
	const std::string & meshFile = m_propLibraries.at (libraryName)->groups ().at (groupName).meshes.at (propName).file;
	return * m_propMeshResources.at (libraryName).at (meshFile);
}

template <class PropCPUResourceManagerBackend>
const PropCPUResourceManager <PropCPUResourceManagerBackend>::PropTextureResource & PropCPUResourceManager <PropCPUResourceManagerBackend>::getTextureResource (const std::string & libraryName, const std::string & groupName, const std::string & propMeshName, const std::string & textureName) const {
	const PropLibrary & library = * m_propLibraries.at (libraryName);

	if (false == textureName.empty ()) {
		const std::string & textureFile = library.groups ().at (groupName).meshes.at (propMeshName).textures.at (textureName);
		return * m_propTextureResources.at (libraryName).at (library.getActualTextureFileName (textureFile));
	}
	else {
		const std::string & meshFile = m_propLibraries.at (libraryName)->groups ().at (groupName).meshes.at (propMeshName).file;
		return * m_propTextureResources.at (libraryName).at (library.getActualTextureFileName (m_propMeshResources.at (libraryName).at (meshFile)->meta->textureFile));
	}
}

template <class PropCPUResourceManagerBackend>
const PropCPUResourceManager <PropCPUResourceManagerBackend>::PropTextureResource & PropCPUResourceManager <PropCPUResourceManagerBackend>::getTextureResource (const std::string & libraryName, const std::string & groupName, const std::string & propSpriteName) const {
	const PropLibrary & library = * m_propLibraries.at (libraryName);

	return * m_propTextureResources.at (libraryName).at (library.getActualTextureFileName (library.groups ().at (groupName).sprites.at (propSpriteName).diffuseFile));
}

template <class PropCPUResourceManagerBackend>
PropCPUResourceManager <PropCPUResourceManagerBackend>::MeshLoaderTask & PropCPUResourceManager <PropCPUResourceManagerBackend>::meshLoader () {
	return m_meshLoader;
}

template <class PropCPUResourceManagerBackend>
PropCPUResourceManager <PropCPUResourceManagerBackend>::TextureLoaderTask & PropCPUResourceManager <PropCPUResourceManagerBackend>::textureLoader () {
	return m_textureLoader;
}

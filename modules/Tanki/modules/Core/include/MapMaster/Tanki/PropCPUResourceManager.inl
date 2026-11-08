# include "MapMaster/Tanki/PropCPUResourceManager.hpp"

# include <algorithm>
# include <compare>
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <execution>
# include <functional>
# include <initializer_list>
# include <map>
# include <memory>
# include <optional>
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

namespace MapMaster::Tanki {

template <class PropCPUResourceManagerBackend>
PropCPUResourceManager <PropCPUResourceManagerBackend>::PropCPUResourceManager (const ResourceSettings & resourceSettings)
	: m_resourceSettings (resourceSettings)
	, m_meshLoader (* this, & PropCPUResourceManager::loadMeshResource)
	, m_textureLoader (* this, & PropCPUResourceManager::loadTextureResource)
	, m_spriteGenerator (* this, & PropCPUResourceManager::generateSpriteResource)
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
	m_propSpriteResources.clear ();
}

template <class PropCPUResourceManagerBackend>
void PropCPUResourceManager <PropCPUResourceManagerBackend>::removePropLibrary (const std::string & name) {
	m_propLibraries.erase (name);
}

template <class PropCPUResourceManagerBackend>
void PropCPUResourceManager <PropCPUResourceManagerBackend>::dropPropLibraryResources (const std::string & name) {
	m_propMeshResources.erase (name);
	m_propTextureResources.erase (name);
	m_propSpriteResources.erase (name);
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
void PropCPUResourceManager <PropCPUResourceManagerBackend>::loadMeshResources (
	const std::vector <std::tuple <std::string, std::string>> & meshDescriptors,
	std::optional <std::reference_wrapper <std::vector <std::shared_ptr <PropMetaData::Mesh>>>> meta
) {
	if (false == m_resourceSettings.dropCPUResources) {
		if (true == meta.has_value ()) {
			throw std::runtime_error ("don't do this. use the metadata from the internal mesh data instead");
		}

		std::vector <std::shared_ptr <PropMeshResource>> resources = m_meshLoader.template run <true, true> (meshDescriptors);

		std::size_t mI = 0;

		for (const auto & [libraryName, meshFile] : meshDescriptors) {
			m_propMeshResources [libraryName] [meshFile] = std::move (resources [mI]);

			mI++;
		}
	}
	else {
		if (true == meta.has_value ()) {
			meta.value ().get () = m_meshLoader.template run <
				true,
				true,
				[] (const std::shared_ptr <PropMeshResource> & meshResource) -> std::shared_ptr <PropMetaData::Mesh> {
					return meshResource->meta;
				}
			> (meshDescriptors);
		}
		else {
			m_meshLoader.template run <false, true> (meshDescriptors);
		}
	}
}

template <class PropCPUResourceManagerBackend>
// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
void PropCPUResourceManager <PropCPUResourceManagerBackend>::loadTextureResources (
	const std::vector <std::tuple <std::string, std::string, std::string>> & textureDescriptors,
	std::optional <std::reference_wrapper <std::vector <std::shared_ptr <PropMetaData::Texture>>>> meta
) {
	if (false == m_resourceSettings.dropCPUResources) {
		if (true == meta.has_value ()) {
			throw std::runtime_error ("don't do this. use the metadata from the internal mesh data instead");
		}

		std::vector <std::shared_ptr <PropTextureResource>> resources = m_textureLoader.template run <true, true> (textureDescriptors);

		std::size_t tI = 0;
		for (const auto & [libraryName, diffuseFile, _] : textureDescriptors) {
			m_propTextureResources [libraryName] [diffuseFile] = std::move (resources [tI]);

			tI++;
		}
	}
	else {
		if (true == meta.has_value ()) {
			meta.value ().get () = m_textureLoader.template run <
				true,
				true,
				[] (const std::shared_ptr <PropTextureResource> & textureResource) -> std::shared_ptr <PropMetaData::Texture> {
					return textureResource->meta;
				}
			> (textureDescriptors);
		}
		else {
			m_textureLoader.template run <false, true> (textureDescriptors);
		}
	}
}

template <class PropCPUResourceManagerBackend>
void PropCPUResourceManager <PropCPUResourceManagerBackend>::generateSpriteResources (
	const std::vector <std::tuple <std::string, std::string, double, double, double, double, double>> & spriteDescriptors
) {
	if (false == m_resourceSettings.dropCPUResources) {
		std::vector <std::shared_ptr <PropSpriteResource>> resources = m_spriteGenerator.template run <true, true> (spriteDescriptors);

		std::size_t sI = 0;
		for (const auto & [libraryName, diffuseFile, _1, _2, _3, _4, _5] : spriteDescriptors) {
			m_propSpriteResources [libraryName] [diffuseFile] = std::move (resources [sI]);
		}
	}
	else {
		m_spriteGenerator.template run <false, true> (spriteDescriptors);
	}
}

template <class PropCPUResourceManagerBackend>
void PropCPUResourceManager <PropCPUResourceManagerBackend>::loadMapResources (const Map & map) {
	std::vector <MeshDescriptor> meshDescriptors;
	std::vector <TextureDescriptor> textureDescriptors;

	std::vector <std::tuple <std::string, std::string, const PropLibrary::PropSprite *>> spritePropDescriptors;
	std::vector <std::tuple <std::string, std::string>> defaultTextureMeshDescriptors;

	std::map <std::string, std::vector <std::tuple <std::string, const PropLibrary::PropSprite *>>> spriteProps;
	std::map <std::string, std::vector <std::string>> defaultTexturesMeshes;

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
							if (true == m_resourceSettings.dropCPUResources) {
								defaultTextureMeshDescriptors.emplace_back (libraryName, mIt->second.file);
							}
							else {
								defaultTexturesMeshes [libraryName].push_back (mIt->second.file);
							}
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

					if (true == m_resourceSettings.dropCPUResources) {
						spritePropDescriptors.emplace_back (libraryName, diffuseFile, & sprite);
					}
					else {
						spriteProps [libraryName].emplace_back (diffuseFile, & sprite);
					}
				}
			}
		}
	}

	if (true == m_resourceSettings.dropCPUResources) {
		loadResourcesAndDrop (
			std::move (meshDescriptors),
			std::move (textureDescriptors),
			std::move (spritePropDescriptors),
			std::move (defaultTextureMeshDescriptors)
		);
	}
	else {
		loadResources (
			std::move (meshDescriptors),
			std::move (textureDescriptors),
			std::move (spriteProps),
			std::move (defaultTexturesMeshes)
		);
	}

	// TODO: add dropResources support for sprite resources
	// TODO: load collision primitives
}

template <class PropCPUResourceManagerBackend>
void PropCPUResourceManager <PropCPUResourceManagerBackend>::loadPropLibraryResources (const std::optional <std::string> & libraryName) {
	if (true == libraryName.has_value ()) {
		// ...
	}
	else {
		for (const auto & [libraryName, library] : m_propLibraries) {
			loadPropLibraryResources (libraryName);
		}
	}
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

	PropMeshResource resource = PropCPUResourceManagerBackend::ParseMeshResource (scene);

	if (true == m_resourceSettings.loadPropColliderData) {
		resource.meta->collider = PropMetaData::Mesh::ParseCollider (scene);
	}

	return resource;
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

template <class PropCPUResourceManagerBackend>
PropCPUResourceManager <PropCPUResourceManagerBackend>::PropSpriteResource PropCPUResourceManager <PropCPUResourceManagerBackend>::generateSpriteResource (const std::string & libraryName, const std::string & diffuseFile, const double & width, const double & height, const double & originX, const double & originY, const double & scale) const {
	return PropCPUResourceManagerBackend::GenerateSpriteResource (
		width,
		height,
		originX,
		originY,
		scale
	);
}

template <class PropCPUResourceManagerBackend>
void PropCPUResourceManager <PropCPUResourceManagerBackend>::loadResourcesAndDrop (
	std::vector <MeshDescriptor> && meshDescriptors,
	std::vector <TextureDescriptor> && textureDescriptors,
	std::vector <std::tuple <std::string, std::string, const PropLibrary::PropSprite *>> && spritePropDescriptors,
	std::vector <std::tuple <std::string, std::string>> && defaultTextureMeshDescriptors
) {
	std::sort (
		std::execution::par_unseq,
		meshDescriptors.begin (),
		meshDescriptors.end ()
	);
	meshDescriptors.erase (
		std::unique (
			std::execution::par_unseq,
			meshDescriptors.begin (),
			meshDescriptors.end ()
		),
		meshDescriptors.end ()
	);

	std::vector <std::shared_ptr <PropMetaData::Mesh>> meshResourceMetaDatas;
	loadMeshResources (meshDescriptors, meshResourceMetaDatas);

	std::sort (
		std::execution::par_unseq,
		defaultTextureMeshDescriptors.begin (),
		defaultTextureMeshDescriptors.end ()
	);
	defaultTextureMeshDescriptors.erase (
		std::unique (
			std::execution::par_unseq,
			defaultTextureMeshDescriptors.begin (),
			defaultTextureMeshDescriptors.end ()
		),
		defaultTextureMeshDescriptors.end ()
	);

	std::size_t mdI = 0;

	for (const auto & defaultTextureMeshDescriptor : defaultTextureMeshDescriptors) {
		while (mdI < meshDescriptors.size ()) {
			if (meshDescriptors.at (mdI) < defaultTextureMeshDescriptor) {
				mdI++;
			}
			else {
				break;
			}
		}

		const std::string & libraryName = std::get <0> (meshDescriptors.at (mdI));
		const PropLibrary & library = * m_propLibraries.at (libraryName);
		std::string diffuseFile = meshResourceMetaDatas.at (mdI)->textureFile;
		std::string alphaFile;

		if (auto it = library.alphaMap ().find (diffuseFile); it != library.alphaMap ().end ()) {
			alphaFile = it->second;
		}
		if (auto it = library.diffuseMap ().find (diffuseFile); it != library.diffuseMap ().end ()) {
			diffuseFile = it->second;
		}
		textureDescriptors.emplace_back (libraryName, diffuseFile, alphaFile);
	}

	std::sort (
		std::execution::par_unseq,
		textureDescriptors.begin (),
		textureDescriptors.end ()
	);
	textureDescriptors.erase (
		std::unique (
			std::execution::par_unseq,
			textureDescriptors.begin (),
			textureDescriptors.end ()
		),
		textureDescriptors.end ()
	);

	std::vector <std::shared_ptr <PropMetaData::Texture>> textureResourceMetaDatas;
	loadTextureResources (textureDescriptors, textureResourceMetaDatas);

	std::sort (
		std::execution::par_unseq,
		spritePropDescriptors.begin (),
		spritePropDescriptors.end ()
	);
	spritePropDescriptors.erase (
		std::unique (
			std::execution::par_unseq,
			spritePropDescriptors.begin (),
			spritePropDescriptors.end ()
		),
		spritePropDescriptors.end ()
	);

	std::vector <SpriteDescriptor> spriteDescriptors;

	std::size_t tdI = 0;

	for (const auto & spritePropDescriptor : spritePropDescriptors) {
		const std::string & libraryName = std::get <0> (spritePropDescriptor);
		const std::string & diffuseFile = std::get <1> (spritePropDescriptor);
		const PropLibrary::PropSprite & spriteInfo = * std::get <2> (spritePropDescriptor);

		while (tdI < textureDescriptors.size ()) {
			const auto & td = textureDescriptors.at (tdI);

			const std::string & tdLibraryName = std::get <0> (td);
			const std::string & tdDiffuseFile = std::get <1> (td);

			if (
				tdLibraryName < libraryName ||
				(tdLibraryName == libraryName && tdDiffuseFile < diffuseFile)
			) {
				tdI++;
			}
			else {
				break;
			}
		}

		const std::shared_ptr <PropMetaData::Texture> & metaData = textureResourceMetaDatas.at (tdI);
		spriteDescriptors.emplace_back (
			libraryName
			, diffuseFile
			, metaData->width
			, metaData->height
			, spriteInfo.originX
			, spriteInfo.originY
			, spriteInfo.scale
		);
	}

	generateSpriteResources (spriteDescriptors);
}

template <class PropCPUResourceManagerBackend>
void PropCPUResourceManager <PropCPUResourceManagerBackend>::loadResources (
	std::vector <MeshDescriptor> && meshDescriptors,
	std::vector <TextureDescriptor> && textureDescriptors,
	std::map <std::string, std::vector <std::tuple <std::string, const PropLibrary::PropSprite *>>> && spriteProps,
	std::map <std::string, std::vector <std::string>> && defaultTexturesMeshes
) {
	std::sort (
		std::execution::par_unseq,
		meshDescriptors.begin (),
		meshDescriptors.end ()
	);
	meshDescriptors.erase (
		std::unique (
			std::execution::par_unseq,
			meshDescriptors.begin (),
			meshDescriptors.end ()
		),
		meshDescriptors.end ()
	);

	loadMeshResources (meshDescriptors);
	std::vector <SpriteDescriptor> spriteDescriptors;

	for (auto & [libraryName, meshFileNames] : defaultTexturesMeshes) {
		std::sort (
			std::execution::par_unseq,
			meshFileNames.begin (),
			meshFileNames.end ()
		);
		meshFileNames.erase (
			std::unique (
				std::execution::par_unseq,
				meshFileNames.begin (),
				meshFileNames.end ()
			),
			meshFileNames.end ()
		);

		const PropLibrary & library = * m_propLibraries.at (libraryName);
		const std::map <std::string, std::shared_ptr <PropMeshResource>> & libraryMeshResources = m_propMeshResources.at (libraryName);

		for (const std::string & meshFileName : meshFileNames) {
			std::string diffuseFile = libraryMeshResources.at (meshFileName)->meta->textureFile;
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

	std::sort (
		std::execution::par_unseq,
		textureDescriptors.begin (),
		textureDescriptors.end ()
	);
	textureDescriptors.erase (
		std::unique (
			std::execution::par_unseq,
			textureDescriptors.begin (),
			textureDescriptors.end ()
		),
		textureDescriptors.end ()
	);

	loadTextureResources (textureDescriptors);

	for (auto & [libraryName, textureDiffuseFiles] : spriteProps) {
		std::sort (
			std::execution::par_unseq,
			textureDiffuseFiles.begin (),
			textureDiffuseFiles.end ()
		);
		textureDiffuseFiles.erase (
			std::unique (
				std::execution::par_unseq,
				textureDiffuseFiles.begin (),
				textureDiffuseFiles.end ()
			),
			textureDiffuseFiles.end ()
		);

		const std::map <std::string, std::shared_ptr <PropTextureResource>> & libraryTextureResources = m_propTextureResources.at (libraryName);

		for (const auto & [textureDiffuseFile, spriteInfo] : textureDiffuseFiles) {
			std::shared_ptr <PropMetaData::Texture> metaData = libraryTextureResources.at (textureDiffuseFile)->meta;
			spriteDescriptors.emplace_back (
				libraryName
				, textureDiffuseFile
				, metaData->width
				, metaData->height
				, spriteInfo->originX
				, spriteInfo->originY
				, spriteInfo->scale
			);
		}
	}

	generateSpriteResources (spriteDescriptors);
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

template <class PropCPUResourceManagerBackend>
PropCPUResourceManager <PropCPUResourceManagerBackend>::SpriteGeneratorTask & PropCPUResourceManager <PropCPUResourceManagerBackend>::spriteGenerator () {
	return m_spriteGenerator;
}

}  // namespace MapMaster::Tanki

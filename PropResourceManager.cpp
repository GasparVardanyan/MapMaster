# include "PropResourceManager.hpp"

# include <algorithm>
# include <cctype>
# include <cstring>
# include <exception>
# include <execution>
# include <iostream>
# include <map>
# include <memory>
# include <set>
# include <string>
# include <utility>
# include <vector>

# include <assimp/Importer.hpp>
# include <assimp/config.h>
# include <assimp/material.h>
# include <assimp/material.inl>
# include <assimp/mesh.h>
# include <assimp/postprocess.h>
# include <assimp/scene.h>
# include <assimp/types.h>
# include <stb_image.h>

# include "Map.hpp"
# include "PropLibrary.hpp"

void PropResourceManager::addPropLibrary (const PropLibrary & propLibrary) {
	m_propLibraries.try_emplace (
		propLibrary.name (),
		propLibrary
	);
}

void PropResourceManager::addPropLibrary (PropLibrary && propLibrary) {
	std::string name = propLibrary.name ();
	m_propLibraries.try_emplace (
		std::move (name),
		std::move (propLibrary)
	);
}

PropResourceManager::PropMeshResource PropResourceManager::loadMeshResources (const std::string & libraryName, const std::string & meshFile) {
	const std::string meshPath = m_propLibraries.at (libraryName).path () + "/" + meshFile;
	std::map <std::string, PropMeshResource> & libraryResources = m_propMeshResources [libraryName];

	Assimp::Importer importer;
	importer.SetPropertyInteger (
		AI_CONFIG_PP_RVC_FLAGS,
		// aiComponent_NORMALS |
		aiComponent_TANGENTS_AND_BITANGENTS |
		aiComponent_COLORS |
		// aiComponent_TEXCOORDS |
		aiComponent_BONEWEIGHTS |
		aiComponent_ANIMATIONS |
		aiComponent_TEXTURES |
		aiComponent_LIGHTS |
		aiComponent_CAMERAS
		// aiComponent_MESHES |
		// aiComponent_MATERIALS
	);

	// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic,readability-math-missing-parentheses)

	const aiScene * scene = importer.ReadFile (meshPath, aiProcess_Triangulate | aiProcess_RemoveComponent | aiProcess_FlipUVs);
	for (std::size_t i = 0; i < scene->mNumMeshes; i++) {
		aiMesh * _mesh = scene->mMeshes [i];
		std::string meshName = _mesh->mName.C_Str ();
		std::ranges::transform (meshName, meshName.begin (), [] (char c) -> char {
			return static_cast <char> (std::tolower (c));
		});

		if (0 == std::strcmp (meshName.c_str (), "occl")) {
			std::cerr << "Found occluder! Unhandled!!" << '\n';
			std::terminate ();
		}
	}

	const aiMesh * mesh = scene->mMeshes [0];
	PropMeshResource resources = libraryResources [meshFile];

	aiString diffuseMapUrl;
	scene->mMaterials [mesh->mMaterialIndex]->GetTexture (aiTextureType_DIFFUSE, 0, & diffuseMapUrl);

	if (false == diffuseMapUrl.Empty ()) {
		std::string matName (diffuseMapUrl.C_Str ());
		std::ranges::transform (matName, matName.begin (), [] (char c) -> char {
			return static_cast <char> (std::tolower (c));
		});

		resources.textureFile = matName;
	}

	resources.vertexBuffer.resize (mesh->mNumVertices * 3UL);
	for (unsigned i = 0; i < mesh->mNumVertices; i++) {
		resources.vertexBuffer [3 * i + 0] = mesh->mVertices [i].x;
		resources.vertexBuffer [3 * i + 1] = mesh->mVertices [i].y;
		resources.vertexBuffer [3 * i + 2] = mesh->mVertices [i].z;
	}

	resources.indexBuffer.resize (mesh->mNumFaces * 3UL);
	for (unsigned i = 0; i < mesh->mNumFaces; ++i) {
		resources.indexBuffer [3 * i + 0] = static_cast<PropMeshResource::IndexType>(mesh->mFaces[i].mIndices[0]);
		resources.indexBuffer [3 * i + 1] = static_cast<PropMeshResource::IndexType>(mesh->mFaces[i].mIndices[1]);
		resources.indexBuffer [3 * i + 2] = static_cast<PropMeshResource::IndexType>(mesh->mFaces[i].mIndices[2]);
	}

	if (true == mesh->HasTextureCoords (0)) {
		resources.uvBuffer.resize (mesh->mNumVertices * 2UL);
		for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
			resources.uvBuffer [2 * i + 0] = mesh->mTextureCoords[0][i].x;
			resources.uvBuffer [2 * i + 1] = mesh->mTextureCoords[0][i].y;
		}
	}
	if (mesh->HasNormals()) {
		resources.normalBuffer.resize (mesh->mNumVertices * 3UL);
		for (unsigned i = 0; i < mesh->mNumVertices; i++) {
			resources.normalBuffer [3 * i + 0] = mesh->mNormals [i].x;
			resources.normalBuffer [3 * i + 1] = mesh->mNormals [i].y;
			resources.normalBuffer [3 * i + 2] = mesh->mNormals [i].z;
		}
	}

	// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic,readability-math-missing-parentheses)

	return resources;
}

void PropResourceManager::loadMapResources (const Map & map) {
	std::vector <std::pair <std::string, std::string>> meshDescriptors;
	std::vector <std::pair <std::string, std::pair <std::string, std::string>>> textureDescriptors;

	std::map <std::string, std::map <std::string, std::set <std::string>>> defaultTextures;

	for (const auto & [libraryName, groupNames] : map.mapObjects ()) {
		const PropLibrary & library = m_propLibraries.at (libraryName);
		const std::map <std::string, PropLibrary::Group> & groups = library.groups ();

		for (const auto & [groupName, propNames] : groupNames) {
			const PropLibrary::Group & group = groups.at (groupName);

			for (const auto & [propName, propList] : propNames) {
				if (auto mIt = group.meshes.find (propName); group.meshes.end () != mIt) {
					meshDescriptors.emplace_back (
						libraryName,
						mIt->second.file
					);

					for (const Map::MapObject & propObject : propList) {
						if (false == propObject.textureName.empty ()) {
							std::string diffuseFile = mIt->second.textures.at (propObject.textureName);
							std::string opacityFile;

							if (auto it = library.opacityMap ().find (diffuseFile); it != library.opacityMap ().end ()) {
								opacityFile = it->second;
							}
							if (auto it = library.diffuseMap ().find (diffuseFile); it != library.diffuseMap ().end ()) {
								diffuseFile = it->second;
							}
							textureDescriptors.emplace_back (
								libraryName,
								std::pair <std::string, std::string> {diffuseFile, opacityFile}
							);
						}
						else {
							defaultTextures [libraryName] [groupName].insert (propName);
						}
					}
				}
				else if (auto sIt = group.sprites.find (propName); group.sprites.end () != sIt) {
					std::string diffuseFile = sIt->second.diffuseFile;
					std::string opacityFile;

					if (library.opacityMap ().contains (diffuseFile)) {
						std::cerr << "Found opacity map! Unhandled!!" << '\n';
						std::terminate ();
					}

					if (auto it = library.opacityMap ().find (diffuseFile); it != library.opacityMap ().end ()) {
						opacityFile = it->second;
					}
					if (auto it = library.diffuseMap ().find (diffuseFile); it != library.diffuseMap ().end ()) {
						diffuseFile = it->second;
					}

					textureDescriptors.emplace_back (
						libraryName,
						std::pair <std::string, std::string> {diffuseFile, opacityFile}
					);
				}
			}
		}
	}

	std::sort (std::execution::par_unseq, meshDescriptors.begin (), meshDescriptors.end ());
	meshDescriptors.erase (std::unique (std::execution::par_unseq, meshDescriptors.begin (), meshDescriptors.end ()), meshDescriptors.end ());

	loadMeshResources (meshDescriptors);

	if (nullptr != m_mapMeshResourcesLoadCallback) {
		m_mapMeshResourcesLoadCallback ();
	}

	for (const auto & [libraryName, groupData] : defaultTextures) {
		const PropLibrary & library = m_propLibraries.at (libraryName);
		const std::map <std::string, PropLibrary::Group> & groups = library.groups ();
		const std::map <std::string, PropMeshResource> & libraryMeshResources = m_propMeshResources.at (libraryName);

		for (const auto & [groupName, props] : groupData) {
			const PropLibrary::Group & group = groups.at (groupName);

			for (const std::string & propName : props) {
				const PropLibrary::PropMesh & prop = group.meshes.at (propName);

				std::string diffuseFile = libraryMeshResources.at (prop.file).textureFile;
				std::string opacityFile;

				if (auto it = library.opacityMap ().find (diffuseFile); it != library.opacityMap ().end ()) {
					opacityFile = it->second;
				}
				if (auto it = library.diffuseMap ().find (diffuseFile); it != library.diffuseMap ().end ()) {
					diffuseFile = it->second;
				}
				textureDescriptors.emplace_back (
					libraryName,
					std::pair <std::string, std::string> {diffuseFile, opacityFile}
				);
			}
		}
	}

	std::sort (std::execution::par_unseq, textureDescriptors.begin (), textureDescriptors.end ());
	textureDescriptors.erase (std::unique (std::execution::par_unseq, textureDescriptors.begin (), textureDescriptors.end ()), textureDescriptors.end ());

	loadTextureResources (textureDescriptors);

	if (nullptr != m_mapTextureResourcesLoadCallback) {
		m_mapTextureResourcesLoadCallback ();
	}
}

PropResourceManager::PropTextureResource PropResourceManager::loadTextureResources (const std::string & libraryName, const std::string & diffuseFile, const std::string & opacityFile) {
	const std::string diffusePath = m_propLibraries.at (libraryName).path () + "/" + diffuseFile;

	int width = 0;
	int height = 0;
	int channels = 0;

	unsigned char * pixels = stbi_load (
		diffusePath.c_str (),
		&width,
		&height,
		&channels,
		STBI_rgb_alpha
	);

	if (false == opacityFile.empty ()) {
		std::cout << "OPMAPFOUND!!\n";
		std::terminate ();
		// const std::string opacityPath = m_propLibraries.at (libraryName).path () + "/" + opacityFile;
		// int alphaWidth = 0;
		// int alphaHeight = 0;
		// int alphaChannels = 0;
		// std::cout << "ALPHA: " << opacityPath << '\n';
		//
		// unsigned char *alphaPixels = stbi_load(
		// 		opacityPath.c_str (),
		// 		&alphaWidth,
		// 		&alphaHeight,
		// 		&alphaChannels,
		// 		STBI_grey
		// 		);
		//
		// if (alphaPixels == nullptr)
		// {
		// 	TraceLog(LOG_WARNING,
		// 			"STB: failed to load alpha image %s: %s",
		// 			opacityPath.c_str (),
		// 			stbi_failure_reason());
		// }
		// else
		// {
		// 	if (alphaWidth != width || alphaHeight != height)
		// 	{
		// 		TraceLog(LOG_WARNING,
		// 				"STB: alpha image size mismatch (%dx%d vs %dx%d)",
		// 				alphaWidth, alphaHeight,
		// 				width, height);
		// 	}
		// 	else
		// 	{
		// 		for (int i = 0; i < width * height; i++)
		// 		{
		// 			pixels[i * 4 + 3] = alphaPixels[i];
		// 		}
		// 	}
		//
		// 	stbi_image_free(alphaPixels);
		// }
	}

	return {
		.pixBuffer = std::shared_ptr <unsigned char> (pixels, stbi_image_free),
		.width = width,
		.height = height,
		.channels = channels
	};
}

void PropResourceManager::loadMeshResources (const std::vector <std::pair <std::string, std::string>> & meshDescriptors) {
	std::vector <PropMeshResource> resources;
	resources.resize (meshDescriptors.size ());

	std::transform (
		std::execution::par_unseq,
		meshDescriptors.cbegin (),
		meshDescriptors.cend (),
		resources.begin (),
		[this] (const std::pair <std::string, std::string> & descriptor) {
			return loadMeshResources (descriptor.first, descriptor.second);
		}
	);

	std::size_t mI = 0;

	for (const std::pair <std::string, std::string> & descriptor : meshDescriptors) {
		m_propMeshResources [descriptor.first] [descriptor.second] = std::move (resources [mI]);
		if (nullptr != m_meshResourceLoadCallback) {
			m_meshResourceLoadCallback (descriptor.first, descriptor.second);
		}
		mI++;
	}
}

void PropResourceManager::loadTextureResources (const std::vector <std::pair <std::string, std::pair <std::string, std::string>>> & textureDescriptors) {
	std::vector <PropTextureResource> resources;
	resources.resize (textureDescriptors.size ());

	std::transform (
		std::execution::par_unseq,
		textureDescriptors.cbegin (),
		textureDescriptors.cend (),
		resources.begin (),
		[this] (const std::pair <std::string, std::pair <std::string, std::string>> & descriptor) {
			return loadTextureResources (descriptor.first, descriptor.second.first, descriptor.second.second);
		}
	);

	std::size_t tI = 0;
	for (const std::pair <std::string, std::pair <std::string, std::string>> & descriptor : textureDescriptors) {
		m_propTextureResources [descriptor.first] [descriptor.second.first] = std::move (resources [tI]);
		if (nullptr != m_textureResourceLoadCallback) {
			m_textureResourceLoadCallback (descriptor.first, descriptor.second.first);
		}
		tI++;
	}
}

const std::map <std::string, PropLibrary> & PropResourceManager::propLibraries () const {
	return m_propLibraries;
}

const PropResourceManager::PropMeshResource & PropResourceManager::getMeshResource (const std::string & libraryName, const std::string & groupName, const std::string & propName) const {
	const std::string & meshFile = m_propLibraries.at (libraryName).groups ().at (groupName).meshes.at (propName).file;
	return m_propMeshResources.at (libraryName).at (meshFile);
}

const PropResourceManager::PropTextureResource & PropResourceManager::getTextureResource (const std::string & libraryName, const std::string & groupName, const std::string & propMeshName, const std::string & textureName) const {
	const PropLibrary & library = m_propLibraries.at (libraryName);

	if (false == textureName.empty ()) {
		const std::string & textureFile = library.groups ().at (groupName).meshes.at (propMeshName).textures.at (textureName);
		return m_propTextureResources.at (libraryName).at (library.actualTextureFile (textureFile));
	}
	else {
		const std::string & meshFile = m_propLibraries.at (libraryName).groups ().at (groupName).meshes.at (propMeshName).file;
		return m_propTextureResources.at (libraryName).at (library.actualTextureFile (m_propMeshResources.at (libraryName).at (meshFile).textureFile));
	}
}

const PropResourceManager::PropTextureResource & PropResourceManager::getTextureResource (const std::string & libraryName, const std::string & groupName, const std::string & propSpriteName) const {
	const PropLibrary & library = m_propLibraries.at (libraryName);

	return m_propTextureResources.at (libraryName).at (library.actualTextureFile(library.groups ().at (groupName).sprites.at (propSpriteName).diffuseFile));
}

const std::map <std::string, std::map <std::string, PropResourceManager::PropMeshResource>> & PropResourceManager::propMeshResources () const {
	return m_propMeshResources;
}

const std::map <std::string, std::map <std::string, PropResourceManager::PropTextureResource>> & PropResourceManager::propTextureResources () const {
	return m_propTextureResources;
}

void PropResourceManager::setMeshResourceLoadCallback (const ResourceLoadCallback & callback) {
	m_meshResourceLoadCallback = callback;
}
void PropResourceManager::setTextureResourceLoadCallback (const ResourceLoadCallback & callback) {
	m_textureResourceLoadCallback = callback;
}

void PropResourceManager::setMapMeshResourcesLoadCallback (const MapResourcesLoadCallback & callback) {
	m_mapMeshResourcesLoadCallback = callback;
}

void PropResourceManager::setMapTextureResourcesLoadCallback(const MapResourcesLoadCallback & callback) {
	m_mapTextureResourcesLoadCallback = callback;
}

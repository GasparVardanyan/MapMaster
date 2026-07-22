# include "PropResourceManager.hpp"

# include <algorithm>
# include <cctype>
# include <cstring>
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

// void PropResourceManager::loadResources (const std::string & libraryName, const std::string & groupName, const std::string & propName) {
// 	const auto & library = m_propLibraries.at (libraryName);
// 	const auto & group = library.groups ().at (groupName);
//
//
// 	// if (true == group.meshes.contains (propName)) {
// 	// 	const auto & propMesh = group.meshes.at (propName);
// 	// 	auto & libraryResources = m_propMeshResources [libraryName];
// 	//
// 	// 	if (false == libraryResources.contains (propMesh.file)) {
// 	// 		m_propMeshResources [libraryName] [propMesh.file] = loadMeshResources (libraryName, propMesh.file);
// 	// 	}
// 	//
// 	// 	m_propResources [libraryName] [groupName].meshResources [propName] = libraryResources.at (propMesh.file);
// 	// }
// 	// else if (true == group.sprites.contains (propName)) {
// 	// 	PropSpriteResource & resources = m_propResources [libraryName] [groupName].spriteResources [propName];
// 	// 	(void) resources;
// 	// }
// }

PropResourceManager::PropMeshResource PropResourceManager::loadMeshResources (const std::string & libraryName, const std::string & meshFile) {
	const std::string meshPath = m_propLibraries.at (libraryName).path () + "/" + meshFile;
	auto & libraryResources = m_propMeshResources [libraryName];

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
			std::cout << "Found occluder" << '\n';
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

void PropResourceManager::loadResources (const std::map <std::string, std::map <std::string, std::vector <std::string>>> & propHierarchy) {
	std::set <std::pair <std::string, std::string>> meshDescriptors;
	std::set <std::pair <std::string, std::string>> textureDescriptors;

	for (const auto & [libraryName, groups] : propHierarchy) {
		const PropLibrary & library = m_propLibraries.at (libraryName);

		for (const auto & [groupName, props] : groups) {
			const auto & group = library.groups ().at (groupName);

			for (const std::string & propName : props) {
				if (auto mIt = group.meshes.find (propName); group.meshes.end () != mIt) {
					meshDescriptors.emplace (
						libraryName,
						mIt->second.file
					);
				}
				else if (true == group.sprites.contains (propName)) {
				}
			}
		}
	}

	loadMeshResources (meshDescriptors);
}

void PropResourceManager::loadMapResources (const Map & map) {
	std::set <std::pair <std::string, std::string>> meshDescriptors;
	std::set <std::pair <std::string, std::pair <std::string, std::string>>> textureDescriptors;

	std::map <std::string, std::map <std::string, std::set <std::string>>> defaultTextures;

	for (const auto & [libraryName, groups] : map.mapObjects ()) {
		const PropLibrary & library = m_propLibraries.at (libraryName);

		for (const auto & [groupName, props] : groups) {
			const auto & group = library.groups ().at (groupName);

			for (const auto & [propName, propList] : props) {
				if (auto mIt = group.meshes.find (propName); group.meshes.end () != mIt) {
					meshDescriptors.emplace (
						libraryName,
						mIt->second.file
					);

					for (const auto & prop : propList) {
						if (false == prop.textureName.empty ()) {
							std::string diffuseFile = mIt->second.textures.at (prop.textureName);
							std::string opacityFile;

							if (auto it = library.opacityMap ().find (diffuseFile); it != library.opacityMap ().end ()) {
								opacityFile = it->second;
							}
							if (auto it = library.diffuseMap ().find (diffuseFile); it != library.diffuseMap ().end ()) {
								diffuseFile = it->second;
							}
							textureDescriptors.emplace (
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

					if (library.opacityMap().contains (diffuseFile)) {
						std::cout << "FFFFFFFFFFFFFFFFFFF\n";
						std::terminate ();
					}

					if (auto it = library.opacityMap ().find (diffuseFile); it != library.opacityMap ().end ()) {
						opacityFile = it->second;
					}
					if (auto it = library.diffuseMap ().find (diffuseFile); it != library.diffuseMap ().end ()) {
						diffuseFile = it->second;
					}

					textureDescriptors.emplace (
						libraryName,
						std::pair <std::string, std::string> {diffuseFile, opacityFile}
					);
				}
			}
		}
	}

	loadMeshResources (meshDescriptors);

	for (const auto & [libraryName, groups] : defaultTextures) {
		const PropLibrary & library = m_propLibraries.at (libraryName);
		const auto & libraryMeshResources = m_propMeshResources.at (libraryName);

		for (const auto & [groupName, props] : groups) {
			const auto & group = library.groups ().at (groupName);

			for (const auto & propName : props) {
				const auto & prop = group.meshes.at (propName);

				std::string diffuseFile = libraryMeshResources.at (prop.file).textureFile;
				std::string opacityFile;

				if (auto it = library.opacityMap ().find (diffuseFile); it != library.opacityMap ().end ()) {
					opacityFile = it->second;
				}
				if (auto it = library.diffuseMap ().find (diffuseFile); it != library.diffuseMap ().end ()) {
					diffuseFile = it->second;
				}
				textureDescriptors.emplace (
					libraryName,
					std::pair <std::string, std::string> {diffuseFile, opacityFile}
				);
			}
		}
	}

	loadTextureResources (textureDescriptors);
}

PropResourceManager::PropTextureResource PropResourceManager::loadTextureResources (const std::string & libraryName, const std::string & diffuseFile, const std::string & opacityFile) {
	const std::string diffusePath = m_propLibraries.at (libraryName).path () + "/" + diffuseFile;
	// const std::string opacityPath = m_propLibraries.at (libraryName).path ();

    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char *pixels = stbi_load(
        diffusePath.c_str (),
        &width,
        &height,
        &channels,
        STBI_rgb_alpha
    );

	if (width == 0 || height == 0) {
		std::cout << "ERR\n";
		std::terminate ();
	}

  //   if (false == opacityPath.empty ())
  //   {
  //       int alphaWidth = 0;
  //       int alphaHeight = 0;
  //       int alphaChannels = 0;
		// std::cout << "ALPHA: " << opacityPath << '\n';
		//
  //       unsigned char *alphaPixels = stbi_load(
  //           opacityPath.c_str (),
  //           &alphaWidth,
  //           &alphaHeight,
  //           &alphaChannels,
  //           STBI_grey
  //       );
		//
  //       if (alphaPixels == nullptr)
  //       {
  //           TraceLog(LOG_WARNING,
  //                    "STB: failed to load alpha image %s: %s",
  //                    opacityPath.c_str (),
  //                    stbi_failure_reason());
  //       }
  //       else
  //       {
  //           if (alphaWidth != width || alphaHeight != height)
  //           {
  //               TraceLog(LOG_WARNING,
  //                        "STB: alpha image size mismatch (%dx%d vs %dx%d)",
  //                        alphaWidth, alphaHeight,
  //                        width, height);
  //           }
  //           else
  //           {
  //               for (int i = 0; i < width * height; i++)
  //               {
  //                   pixels[i * 4 + 3] = alphaPixels[i];
  //               }
  //           }
		//
  //           stbi_image_free(alphaPixels);
  //       }
  //   }
	return {
		.pixBuffer = std::shared_ptr <unsigned char> (pixels, stbi_image_free),
		.width = width,
		.height = height,
		.channels = channels
	};
}

void PropResourceManager::loadMeshResources (const std::set <std::pair <std::string, std::string>> & meshDescriptors) {
	std::vector <PropMeshResource> resources;
	resources.resize (meshDescriptors.size ());

	std::transform (
		std::execution::par_unseq,
		meshDescriptors.cbegin (),
		meshDescriptors.cend (),
		resources.begin (),
		[this] (const auto & descriptor) {
			return loadMeshResources (descriptor.first, descriptor.second);
		}
	);

	std::size_t mI = 0;

	for (const auto & descriptor : meshDescriptors) {
		m_propMeshResources [descriptor.first] [descriptor.second] = std::move (resources [mI]);
		mI++;
	}
}

void PropResourceManager::loadTextureResources (const std::set <std::pair <std::string, std::pair <std::string, std::string>>> & textureDescriptors) {
	std::vector <PropTextureResource> resources;
	resources.resize (textureDescriptors.size ());

	// for (const auto & [libraryName, textureInfo] : textureDescriptors) {
	// 	std::cout << "L: " << libraryName << ", T: " << textureInfo.first << ", A: " << textureInfo.second << '\n';
	// }

	std::transform (
		std::execution::par_unseq,
		textureDescriptors.cbegin (),
		textureDescriptors.cend (),
		resources.begin (),
		[this] (const auto & descriptor) {
			return loadTextureResources (descriptor.first, descriptor.second.first, descriptor.second.second);
		}
	);

	std::size_t tI = 0;
	for (const auto & descriptor : textureDescriptors) {
		m_propTextureResources [descriptor.first] [descriptor.second.first] = std::move (resources [tI]);
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

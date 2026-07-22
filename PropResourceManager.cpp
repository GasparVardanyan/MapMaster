# include "PropResourceManager.hpp"

# include <algorithm>
# include <cctype>
# include <cstring>
# include <execution>
# include <iostream>
# include <map>
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

PropResourceManager::PropMeshResource PropResourceManager::loadMeshResources (const std::string & libraryName, const std::string & fileName) {
	const std::string meshPath = m_propLibraries.at (libraryName).path () + "/" + fileName;
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
	PropMeshResource resources = libraryResources [fileName];

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
	// struct MeshResourceDescriptor {
	// 	std::string libraryName;
	// 	std::string meshFile;
	// 	bool operator= (const MeshResourceDescriptor & other) const = default;
	// };
	//
	// std::unordered_set <MeshResourceDescriptor> resourcesToLoad;
	std::set <std::pair <std::string, std::string>> resourceSet;

	for (const auto & [libraryName, groups] : propHierarchy) {
		const PropLibrary & library = m_propLibraries.at (libraryName);

		for (const auto & [groupName, props] : groups) {
			const auto & group = library.groups ().at (groupName);

			for (const std::string & propName : props) {
				if (true == group.meshes.contains (propName)) {
					resourceSet.insert ({
						libraryName,
						group.meshes.at (propName).file,
					});
				}
				else if (true == group.sprites.contains (propName)) {
				}
			}
		}
	}

	std::vector <std::pair <std::string, std::string>> resourcesToLoad (resourceSet.cbegin (), resourceSet.cend ());

	std::vector <PropMeshResource> resources;
	resources.resize (resourcesToLoad.size ());

	std::transform (std::execution::par_unseq, resourcesToLoad.cbegin (), resourcesToLoad.cend (), resources.begin (), [this] (const auto & inf) {
		return loadMeshResources (inf.first, inf.second);
	});

	for (std::size_t rI = 0; rI < resourcesToLoad.size (); rI++) {
		auto & resourceDescriptor = resourcesToLoad [rI];
		m_propMeshResources [std::move (resourceDescriptor.first)] [std::move (resourceDescriptor.second)] = std::move (resources [rI]);
	}
}

const std::map <std::string, PropLibrary> & PropResourceManager::propLibraries () const {
	return m_propLibraries;
}

const PropResourceManager::PropMeshResource & PropResourceManager::getMeshResource(const std::string & libraryName, const std::string & groupName, const std::string & propName) const {
	const std::string & meshFile = m_propLibraries.at (libraryName).groups ().at (groupName).meshes.at (propName).file;
	return m_propMeshResources.at (libraryName).at (meshFile);
}

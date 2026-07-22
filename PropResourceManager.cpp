# include "PropResourceManager.hpp"

# include <algorithm>
# include <assimp/Importer.hpp>
# include <assimp/config.h>
# include <assimp/material.h>
# include <assimp/mesh.h>
# include <assimp/postprocess.h>
# include <assimp/scene.h>
# include <assimp/types.h>
# include <cctype>
# include <cstring>
# include <iostream>
# include <map>
# include <string>
# include <utility>



// FIXME: store mesh resources based on mesh files, not prop names


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

void PropResourceManager::loadResources (const std::string & libraryName, const std::string & groupName, const std::string & propName) {
	const auto & library = m_propLibraries.at (libraryName);
	const auto & group = library.groups ().at (groupName);
	const std::string path = m_propLibraries.at (libraryName).path ();

	if (true == group.meshes.contains (propName)) {
		const auto & propMesh = group.meshes.at (propName);

		const std::string meshPath = path + "/" + propMesh.file;

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
		PropMeshResource & resources = m_propResources [libraryName] [groupName].meshResources [propName];

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
	}
	else if (true == group.sprites.contains (propName)) {
		PropSpriteResource & resources = m_propResources [libraryName] [groupName].spriteResources [propName];
		(void) resources;
	}
}

const std::map <std::string, std::map <std::string, PropResourceManager::Group>> & PropResourceManager::propResources () const {
	return m_propResources;
}

const std::map <std::string, PropLibrary> & PropResourceManager::propLibraries () const {
	return m_propLibraries;
}

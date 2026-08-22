# include <algorithm>
# include <cctype>
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <memory>
# include <string>
# include <vector>

# include <assimp/config.h>
# include <assimp/material.h>
# include <assimp/mesh.h>
# include <assimp/postprocess.h>
# include <assimp/scene.h>
# include <assimp/types.h>
# include <stb_image.h>

# include "MapMaster/Tanki/PropCPUResourceManagerRaylibBackend.hpp"

using namespace MapMaster::Tanki;



int PropCPUResourceManagerRaylibBackend::AssimpImporterRemoveComponentFlags =
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
;

unsigned int PropCPUResourceManagerRaylibBackend::AssimpPostProcessorSteps =
	aiProcess_RemoveComponent |
	aiProcess_FlipUVs
;

PropCPUResourceManagerRaylibBackend::PropMeshResource PropCPUResourceManagerRaylibBackend::ParseMeshResource (const aiScene * scene, const aiNode * visualNode) {
	// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic,readability-math-missing-parentheses)

	PropMeshResource meshResource;

	if (1 == visualNode->mNumMeshes) {
		const aiMesh * mesh = scene->mMeshes [visualNode->mMeshes [0]];

		aiString diffuseMapUrl;
		scene->mMaterials [mesh->mMaterialIndex]->GetTexture (aiTextureType_DIFFUSE, 0, & diffuseMapUrl);

		if (false == diffuseMapUrl.Empty ()) {
			std::string matName (diffuseMapUrl.C_Str ());
			std::ranges::transform (matName, matName.begin (), [] (char c) -> char {
				return static_cast <char> (std::tolower (c));
			});

			meshResource.textureFile = matName;
		}

		meshResource.vertexBuffer.resize (mesh->mNumVertices * 3UL);

		for (unsigned i = 0; i < mesh->mNumVertices; i++) {
			meshResource.vertexBuffer [3 * i + 0] = mesh->mVertices [i].x;
			meshResource.vertexBuffer [3 * i + 1] = mesh->mVertices [i].y;
			meshResource.vertexBuffer [3 * i + 2] = mesh->mVertices [i].z;
		}

		meshResource.indexBuffer.resize (mesh->mNumFaces * 3UL);

		for (unsigned i = 0; i < mesh->mNumFaces; ++i) {
			meshResource.indexBuffer [3 * i + 0] = static_cast <PropMeshResource::IndexType> (mesh->mFaces [i].mIndices [0]);
			meshResource.indexBuffer [3 * i + 1] = static_cast <PropMeshResource::IndexType> (mesh->mFaces [i].mIndices [1]);
			meshResource.indexBuffer [3 * i + 2] = static_cast <PropMeshResource::IndexType> (mesh->mFaces [i].mIndices [2]);
		}

		if (true == mesh->HasTextureCoords (0)) {
			meshResource.uvBuffer.resize (mesh->mNumVertices * 2UL);

			for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
				meshResource.uvBuffer [2 * i + 0] = mesh->mTextureCoords [0] [i].x;
				meshResource.uvBuffer [2 * i + 1] = mesh->mTextureCoords [0] [i].y;
			}
		}

		if (true == mesh->HasNormals ()) {
			meshResource.normalBuffer.resize (mesh->mNumVertices * 3UL);

			for (unsigned i = 0; i < mesh->mNumVertices; i++) {
				meshResource.normalBuffer [3 * i + 0] = mesh->mNormals [i].x;
				meshResource.normalBuffer [3 * i + 1] = mesh->mNormals [i].y;
				meshResource.normalBuffer [3 * i + 2] = mesh->mNormals [i].z;
			}
		}
	}
	else {
		std::size_t vertexBufferSize = 0;
		std::size_t vertexBufferOffset = 0;
		std::size_t vertexIndexOffset = 0;
		std::size_t indexBufferSize = 0;
		std::size_t indexBufferOffset = 0;
		std::size_t uvBufferSize = 0;
		std::size_t uvBufferOffset = 0;
		std::size_t normalBufferSize = 0;
		std::size_t normalBufferOffset = 0;

		{
			const aiMesh * mesh = scene->mMeshes [visualNode->mMeshes [0]];
			aiString diffuseMapUrl;
			scene->mMaterials [mesh->mMaterialIndex]->GetTexture (aiTextureType_DIFFUSE, 0, & diffuseMapUrl);

			if (false == diffuseMapUrl.Empty ()) {
				std::string matName (diffuseMapUrl.C_Str ());
				std::ranges::transform (matName, matName.begin (), [] (char c) -> char {
					return static_cast <char> (std::tolower (c));
				});

				meshResource.textureFile = matName;
			}
		}

		for (unsigned int meshI = 0; meshI < visualNode->mNumMeshes; meshI++) {
			const aiMesh * mesh = scene->mMeshes [visualNode->mMeshes [meshI]];
			vertexBufferSize += mesh->mNumVertices * 3UL;
			indexBufferSize += mesh->mNumFaces * 3UL;
			if (true == mesh->HasTextureCoords (0)) {
				uvBufferSize += mesh->mNumVertices * 2UL;
			}
			if (true == mesh->HasNormals ()) {
				normalBufferSize += mesh->mNumVertices * 3UL;
			}
		}

		meshResource.vertexBuffer.resize (vertexBufferSize);
		meshResource.indexBuffer.resize (indexBufferSize);
		meshResource.uvBuffer.resize (uvBufferSize);
		meshResource.normalBuffer.resize (normalBufferSize);

		for (unsigned int meshI = 0; meshI < visualNode->mNumMeshes; meshI++) {
			const aiMesh * mesh = scene->mMeshes [visualNode->mMeshes [meshI]];

			std::size_t currentVertexBufferSize = mesh->mNumVertices * 3UL;

			for (unsigned mI = 0, rI = vertexBufferOffset; mI < mesh->mNumVertices; mI++) {
				meshResource.vertexBuffer [rI++] = mesh->mVertices [mI].x;
				meshResource.vertexBuffer [rI++] = mesh->mVertices [mI].y;
				meshResource.vertexBuffer [rI++] = mesh->mVertices [mI].z;
			}

			std::size_t currentIndexBufferSize = mesh->mNumFaces * 3UL;

			for (unsigned mI = 0, rI = indexBufferOffset; mI < mesh->mNumFaces; mI++) {
				meshResource.indexBuffer [rI++] = static_cast <PropMeshResource::IndexType> (vertexIndexOffset + mesh->mFaces [mI].mIndices [0]);
				meshResource.indexBuffer [rI++] = static_cast <PropMeshResource::IndexType> (vertexIndexOffset + mesh->mFaces [mI].mIndices [1]);
				meshResource.indexBuffer [rI++] = static_cast <PropMeshResource::IndexType> (vertexIndexOffset + mesh->mFaces [mI].mIndices [2]);
			}

			vertexBufferOffset += currentVertexBufferSize;
			vertexIndexOffset += mesh->mNumVertices;
			indexBufferOffset += currentIndexBufferSize;

			// FIXME: what if first mesh doesn't have normals/textcoords and second have? This indexing will fail.

			if (true == mesh->HasTextureCoords (0)) {
				std::size_t currentUVBufferSize = mesh->mNumVertices * 2UL;

				for (unsigned mI = 0, rI = uvBufferOffset; mI < mesh->mNumVertices; mI++) {
					meshResource.uvBuffer [rI++] = mesh->mTextureCoords [0] [mI].x;
					meshResource.uvBuffer [rI++] = mesh->mTextureCoords [0] [mI].y;
				}

				uvBufferOffset += currentUVBufferSize;
			}

			if (true == mesh->HasNormals ()) {
				std::size_t currentNormalBufferSize = currentVertexBufferSize;

				for (unsigned mI = 0, rI = normalBufferOffset; mI < mesh->mNumVertices; mI++) {
					meshResource.normalBuffer [rI++] = mesh->mNormals [mI].x;
					meshResource.normalBuffer [rI++] = mesh->mNormals [mI].y;
					meshResource.normalBuffer [rI++] = mesh->mNormals [mI].z;
				}

				normalBufferOffset += currentNormalBufferSize;
			}
		}
	}

	return meshResource;

	// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic,readability-math-missing-parentheses)
}

PropCPUResourceManagerRaylibBackend::PropTextureResource PropCPUResourceManagerRaylibBackend::ParseTextureResource (std::FILE * diffuseFileHandle, std::FILE * alphaFileHandle) {
	int width = 0;
	int height = 0;
	int channels = 0;
	int desiredChannels = 3;

	if (nullptr != alphaFileHandle) {
		desiredChannels = 4;
	}

	unsigned char * pixels = stbi_load_from_file (
		diffuseFileHandle,
		& width,
		& height,
		& channels,
		desiredChannels
	);

	if (nullptr != alphaFileHandle) {
		int alphaWidth = 0;
		int alphaHeight = 0;
		int alphaChannels = 0;

		unsigned char * alphaPixels = stbi_load_from_file (
			alphaFileHandle,
			& alphaWidth,
			& alphaHeight,
			& alphaChannels,
			STBI_grey
		);

		for (int i = 0; i < width * height; i++) {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic,readability-math-missing-parentheses)
			pixels [i * 4 + 3] = alphaPixels [i];
		}

		stbi_image_free (alphaPixels);
	}

	return {
		.pixBuffer = std::shared_ptr <unsigned char> (pixels, stbi_image_free),
		.width = width,
		.height = height,
		.channels = desiredChannels
	};
}



PropCPUResourceManagerRaylibBackend::PropTextureResource PropCPUResourceManagerRaylibBackend::PropTextureResource::clone () {
	if (width < 0 || height < 0 || channels < 0) {
		return {};
	}
	else {
		std::size_t bufSize = static_cast <std::size_t> (width) * height * channels;
		// NOLINTNEXTLINE(hicpp-use-auto,modernize-use-auto,cppcoreguidelines-owning-memory,hicpp-no-malloc,cppcoreguidelines-no-malloc)
		unsigned char * newPixBuf = static_cast <unsigned char *> (std::malloc (bufSize));
		std::memcpy (newPixBuf, pixBuffer.get (), bufSize);

		return {
			.pixBuffer = std::shared_ptr <unsigned char> (newPixBuf),
			.width = width,
			.height = height,
			.channels = channels
		};
	}
}

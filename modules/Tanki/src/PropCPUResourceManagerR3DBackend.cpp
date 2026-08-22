# include <algorithm>
# include <cctype>
# include <cstdint>
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <limits>
# include <memory>
# include <string>

# include <assimp/material.h>
# include <assimp/mesh.h>
# include <assimp/scene.h>
# include <assimp/types.h>
# include <stb_image.h>
# include <raylib.h>
# include <raymath.h>
# include <r3d/r3d_mesh_data.h>
# include <r3d/r3d_vertex.h>

# include "MapMaster/Tanki/PropCPUResourceManagerR3DBackend.hpp"

using namespace MapMaster::Tanki;



PropCPUResourceManagerR3DBackend::PropMeshResource PropCPUResourceManagerR3DBackend::ParseMeshResource (const aiScene * scene, const aiNode * visualNode) {
	// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic,readability-math-missing-parentheses)

	PropMeshResource meshResource;
	int vertexCount = 0;
	int indexCount = 0;

	for (std::size_t i = 0; i < visualNode->mNumMeshes; i++) {
		const aiMesh * mesh = scene->mMeshes [visualNode->mMeshes [i]];
		vertexCount += static_cast <int> (mesh->mNumVertices);
		indexCount += static_cast <int> (mesh->mNumFaces * 3UL);
	}

	std::shared_ptr <R3D_MeshData> & meshData = meshResource.mesh;
	meshData = std::shared_ptr <R3D_MeshData> (new R3D_MeshData (R3D_LoadMeshData (
		vertexCount,
		indexCount
	)), [] (R3D_MeshData * meshData) -> void {
		R3D_UnloadMeshData (* meshData);
	});

	meshData->vertexCount = vertexCount;
	meshData->indexCount = indexCount;

	BoundingBox & aabb = meshResource.aabb;
	aabb = {
		.min = {
			.x = std::numeric_limits <float>::max (),
			.y = std::numeric_limits <float>::max (),
			.z = std::numeric_limits <float>::max (),
		},
		.max = {
			.x = std::numeric_limits <float>::min (),
			.y = std::numeric_limits <float>::min (),
			.z = std::numeric_limits <float>::min (),
		},
	};

	R3D_Vertex * vPtr = meshData->vertices;
	uint32_t * iPtr = meshData->indices;
	std::size_t vertexOffset = 0;

	for (std::size_t mI = 0; mI < visualNode->mNumMeshes; mI++) {
		const aiMesh * mesh = scene->mMeshes [visualNode->mMeshes [mI]];

		aiString diffuseMapUrl;
		scene->mMaterials [mesh->mMaterialIndex]->GetTexture (aiTextureType_DIFFUSE, 0, & diffuseMapUrl);

		if (false == diffuseMapUrl.Empty ()) {
			std::string matName (diffuseMapUrl.C_Str ());
			std::ranges::transform (matName, matName.begin (), [] (char c) -> char {
				return static_cast <char> (std::tolower (c));
			});

			meshResource.textureFile = matName;
		}

		const int vertexCount = static_cast <int> (mesh->mNumVertices  * sizeof (* R3D_MeshData::vertices));
		const int indexCount = static_cast <int> (mesh->mNumFaces * 3UL * sizeof (* R3D_MeshData::indices));

		for (unsigned i = 0; i < mesh->mNumVertices; i++) {
			vPtr->position = {
				.x = mesh->mVertices [i].x,
				.y = mesh->mVertices [i].y,
				.z = mesh->mVertices [i].z,
			};

			aabb.min = Vector3Min (aabb.min, vPtr->position);
			aabb.max = Vector3Max (aabb.max, vPtr->position);

			// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
			uint16_t (& texCoord) [2]  = vPtr->texcoord;

			if (nullptr != mesh->mTextureCoords [0] && mesh->mNumUVComponents [0] >= 2) {
				R3D_PackTexCoord (texCoord, (Vector2) {
					.x = mesh->mTextureCoords [0] [i].x,
					.y = mesh->mTextureCoords [0] [i].y,
				});
			}

			// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
			int8_t (& normal) [4] = vPtr->normal;

			if (nullptr != mesh->mNormals) {
				R3D_PackNormal (normal, (Vector3) {
					.x = mesh->mNormals [i].x,
					.y = mesh->mNormals [i].y,
					.z = mesh->mNormals [i].z,
				});
			}

			vPtr->color = WHITE;

			vPtr++;
		}

		for (unsigned i = 0; i < mesh->mNumFaces; i++) {
			* iPtr = vertexOffset + mesh->mFaces [i].mIndices [0];
			iPtr++;
			* iPtr = vertexOffset + mesh->mFaces [i].mIndices [1];
			iPtr++;
			* iPtr = vertexOffset + mesh->mFaces [i].mIndices [2];
			iPtr++;
		}

		vertexOffset += mesh->mNumVertices;
	}

	R3D_GenMeshDataTangents (meshData.get (), R3D_PRIMITIVE_TRIANGLES);

	return meshResource;

	// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic,readability-math-missing-parentheses)
}

PropCPUResourceManagerR3DBackend::PropTextureResource PropCPUResourceManagerR3DBackend::ParseTextureResource (std::FILE * diffuseFileHandle, std::FILE * alphaFileHandle) {
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



PropCPUResourceManagerR3DBackend::PropTextureResource PropCPUResourceManagerR3DBackend::PropTextureResource::clone () {
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

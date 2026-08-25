# include <algorithm>
# include <cctype>
# include <cmath>
# include <cstdint>
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <limits>
# include <memory>
# include <string>

# include <raylib.h>
# include <raymath.h>
# include <r3d/r3d_mesh_data.h>
# include <r3d/r3d_vertex.h>

# include <assimp/config.h>
# include <assimp/material.h>
# include <assimp/mesh.h>
# include <assimp/postprocess.h>
# include <assimp/scene.h>
# include <assimp/types.h>
# include <assimp/vector3.h>
# include <stb_image.h>

# include "MapMaster/Tanki/PropCPUResourceManagerR3DBackend.hpp"
# include "MapMaster/Tanki/PropCPUResourceManager.hpp"
# include "MapMaster/Tanki/PropCPUResourceManager.inl" // IWYU pragma: keep

using namespace MapMaster::Tanki;



int PropCPUResourceManagerR3DBackend::AssimpImporterRemoveComponentFlags =
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

unsigned int PropCPUResourceManagerR3DBackend::AssimpPostProcessorSteps =
	aiProcess_RemoveComponent |
	aiProcess_FlipUVs |
	aiProcess_CalcTangentSpace
;

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

		// NOLINTBEGIN(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
		for (unsigned i = 0; i < mesh->mNumVertices; i++) {
			const aiVector3D & assimpVertex = mesh->mVertices [i];
			const aiVector3D & assimpTexCoord = mesh->mTextureCoords [0] [i];
			const aiVector3D & assimpNormal = mesh->mNormals [i];
			const aiVector3D & assimpTangent = mesh->mTangents [i];
			const aiVector3D & assimpBiTangent = mesh->mBitangents [i];

			vPtr->position = {
				.x = assimpVertex.x,
				.y = assimpVertex.y,
				.z = assimpVertex.z,
			};

			aabb.min = Vector3Min (aabb.min, vPtr->position);
			aabb.max = Vector3Max (aabb.max, vPtr->position);

			uint16_t (& texCoord) [2]  = vPtr->texcoord;

			R3D_PackTexCoord (texCoord, (Vector2) {
				.x = assimpTexCoord.x,
				.y = assimpTexCoord.y,
			});

			int8_t (& normal) [4] = vPtr->normal;

			R3D_PackNormal (normal, (Vector3) {
				.x = assimpNormal.x,
				.y = assimpNormal.y,
				.z = assimpNormal.z,
			});

			int8_t (& tangent) [4] = vPtr->tangent;

			aiVector3D reconstructedBitangent = assimpNormal ^ assimpTangent;
			float handedness = reconstructedBitangent * assimpBiTangent;

			R3D_PackTangent (tangent, (Vector4) {
				.x = assimpTangent.x,
				.y = assimpTangent.y,
				.z = assimpTangent.z,
				.w = std::copysignf (1.0F, handedness)
			});

			vPtr->color = WHITE;

			vPtr++;
		}
		// NOLINTEND(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)

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

namespace MapMaster::Tanki {
template class PropCPUResourceManager <PropCPUResourceManagerR3DBackend>;
} // end namespace MapMaster::Tanki

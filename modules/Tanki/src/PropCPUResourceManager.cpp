# include "MapMaster/Tanki/PropCPUResourceManager.hpp"

# include <algorithm>
# include <array>
# include <cctype>
# include <cstdlib>
# include <cstring>
# include <execution>
# include <functional>
# include <initializer_list>
# include <iterator>
# include <map>
# include <memory>
# include <queue>
# include <set>
# include <stdexcept>
# include <string>
# include <tuple>
# include <utility>
# include <vector>

# include <assimp/Importer.hpp>
# include <assimp/config.h>
# include <assimp/material.h>
# include <assimp/material.inl>
# include <assimp/matrix4x4.h>
# include <assimp/mesh.h>
# include <assimp/postprocess.h>
# include <assimp/scene.h>
# include <assimp/types.h>
# include <assimp/vector3.h>
# include <stb_image.h>

# include "MapMaster/Tanki/Map.hpp"
# include "MapMaster/Tanki/PropLibrary.hpp"

using namespace MapMaster::Tanki;



PropCPUResourceManager::PropCPUResourceManager (bool parseCollisionPrimitives)
	: m_parseCollisionPrimitives (parseCollisionPrimitives)
{
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void PropCPUResourceManager::addPropLibrary (std::shared_ptr <PropLibrary> propLibrary) {
	const std::string & libraryName = propLibrary->name ();
	m_propLibraries.try_emplace (
		libraryName,
		std::move (propLibrary)
	);
}

void PropCPUResourceManager::dropResources () {
	m_propMeshResources.clear ();
	m_propTextureResources.clear ();
}

void PropCPUResourceManager::removePropLibrary (const std::string & name) {
	m_propLibraries.erase (name);
}

void PropCPUResourceManager::clearPropLibraries () {
	m_propLibraries = {};
}

void PropCPUResourceManager::setOverlapBehaviour (OverlapBehaviour overlapBehaviour) {
	throw std::runtime_error ("PropCPUResourceManager::OverlapBehaviour is not in effect");
	m_overlapBehaviour = overlapBehaviour; // cppcheck-suppress unreachableCode
}

PropCPUResourceManager::PropTextureResource PropCPUResourceManager::PropTextureResource::clone () {
	if (width < 0 || height < 0 || channels < 0) {
		return {};
	}
	else {
		std::size_t bufSize = static_cast <std::size_t>(width) * height * channels;
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



//  _      ____          _____  ______ _____   _____
// | |    / __ \   /\   |  __ \|  ____|  __ \ / ____|
// | |   | |  | | /  \  | |  | | |__  | |__) | (___
// | |   | |  | |/ /\ \ | |  | |  __| |  _  / \___ \
// | |___| |__| / ____ \| |__| | |____| | \ \ ____) |
// |______\____/_/    \_\_____/|______|_|  \_\_____/
//

void PropCPUResourceManager::loadMeshResources (const std::vector <std::pair <std::string, std::string>> & meshDescriptors) {
	std::vector <std::pair <std::shared_ptr <PropMeshResource>, std::shared_ptr <Collider>>> resources;
	resources.resize (meshDescriptors.size ());

	std::transform (
		std::execution::par_unseq,
		meshDescriptors.cbegin (),
		meshDescriptors.cend (),
		resources.begin (),
		[this] (const std::pair <std::string, std::string> & descriptor) -> std::pair <std::shared_ptr <PropMeshResource>, std::shared_ptr <Collider>> {
			ParsedMeshInfo meshInfo = loadMeshResource (descriptor.first, descriptor.second);
			std::shared_ptr <Collider> collider = std::make_shared <Collider> (std::move (meshInfo.collider));

			if (1 == meshInfo.meshResources.size ()) {
				std::shared_ptr <PropMeshResource> meshResource = std::make_shared <PropMeshResource> (std::move (meshInfo.meshResources [0]));

				if (nullptr != m_callbacks.meshResourceLoad) {
					m_callbacks.meshResourceLoad (descriptor.first, descriptor.second, meshResource, collider);
				}

				// NOLINTNEXTLINE(google-build-explicit-make-pair)
				return std::make_pair <std::shared_ptr <PropMeshResource>, std::shared_ptr <Collider>> (
					std::move (meshResource),
					std::move (collider)
				);
			}
			else {
				PropMeshResource meshResource {};
				meshResource.textureFile = meshInfo.meshResources [0].textureFile;

				std::size_t vertexBufferSize = 0;
				std::size_t indexBufferSize = 0;
				std::size_t indexBufferOffset = 0;
				std::size_t uvBufferSize = 0;
				std::size_t normalBufferSize = 0;

				for (const PropMeshResource & res : meshInfo.meshResources) {
					vertexBufferSize += res.vertexBuffer.size ();
					indexBufferSize += res.indexBuffer.size ();
					uvBufferSize += res.uvBuffer.size ();
					normalBufferSize += res.normalBuffer.size ();
				}

				meshResource.vertexBuffer.reserve (vertexBufferSize);
				meshResource.indexBuffer.resize (indexBufferSize);
				meshResource.uvBuffer.reserve (uvBufferSize);
				meshResource.normalBuffer.reserve (normalBufferSize);

				for (PropMeshResource & res : meshInfo.meshResources) {
					std::size_t vertexBufferOffset = meshResource.vertexBuffer.size () / 3;

					meshResource.vertexBuffer.insert (
						meshResource.vertexBuffer.end (),
						std::make_move_iterator (res.vertexBuffer.begin ()),
						std::make_move_iterator (res.vertexBuffer.end ())
					);

					std::transform (
						res.indexBuffer.cbegin (),
						res.indexBuffer.cend (),
						meshResource.indexBuffer.begin () + indexBufferOffset,
						[&vertexBufferOffset] (PropMeshResource::IndexType i) -> PropMeshResource::IndexType { return vertexBufferOffset + i; }
					);
					indexBufferOffset += res.indexBuffer.size ();

					meshResource.uvBuffer.insert (
						meshResource.uvBuffer.end (),
						std::make_move_iterator (res.uvBuffer.begin ()),
						std::make_move_iterator (res.uvBuffer.end ())
					);

					meshResource.normalBuffer.insert (
						meshResource.normalBuffer.end (),
						std::make_move_iterator (res.normalBuffer.begin ()),
						std::make_move_iterator (res.normalBuffer.end ())
					);
				}

				std::shared_ptr <PropMeshResource> res = std::make_shared <PropMeshResource> (std::move (meshResource));

				if (nullptr != m_callbacks.meshResourceLoad) {
					m_callbacks.meshResourceLoad (descriptor.first, descriptor.second, res, collider);
				}

				// NOLINTNEXTLINE(google-build-explicit-make-pair)
				return std::make_pair <std::shared_ptr <PropMeshResource>, std::shared_ptr <Collider>> (
					std::move (res),
					std::move (collider)
				);
			}
		}
	);

	std::size_t mI = 0;

	for (const auto & [libraryName, meshFile] : meshDescriptors) {
		auto & [meshResource, collider] = resources [mI];
		m_propMeshResources [libraryName] [meshFile] = std::move (meshResource);
		m_colliders [libraryName] [meshFile] = std::move (collider);

		mI++;
	}
}

void PropCPUResourceManager::loadTextureResources (const std::vector <std::tuple <std::string, std::string, std::string>> & textureDescriptors) {
	std::vector <std::shared_ptr <PropTextureResource>> resources;
	resources.resize (textureDescriptors.size ());

	std::transform (
		std::execution::par_unseq,
		textureDescriptors.cbegin (),
		textureDescriptors.cend (),
		resources.begin (),
		[this] (const std::tuple <std::string, std::string, std::string> & descriptor) -> std::shared_ptr <PropTextureResource> {
			std::shared_ptr <PropTextureResource> textureResource = std::make_shared <PropTextureResource> (loadTextureResource (std::get <0> (descriptor), std::get <1> (descriptor), std::get <2> (descriptor)));

			if (nullptr != m_callbacks.textureResourceLoad) {

				m_callbacks.textureResourceLoad (std::get <0> (descriptor), std::get <1> (descriptor), textureResource);
			}
			return textureResource;
		}
	);

	std::size_t tI = 0;
	for (const auto & [libraryName, diffuseFile, _] : textureDescriptors) {
		m_propTextureResources [libraryName] [diffuseFile] = (std::move (resources [tI]));

		tI++;
	}
}

void PropCPUResourceManager::loadMapResources (const Map & map) {
	std::vector <std::pair <std::string, std::string>> meshDescriptors;
	std::vector <std::tuple <std::string, std::string, std::string>> textureDescriptors;

	std::map <std::string, std::map <std::string, std::set <std::string>>> defaultTextures;

	for (const auto & [libraryName, groupNames] : map.mapObjects ()) {
		const PropLibrary & library = * m_propLibraries.at (libraryName);
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
				else if (auto sIt = group.sprites.find (propName); group.sprites.end () != sIt) {
					std::string diffuseFile = sIt->second.diffuseFile;
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

	loadMeshResources (meshDescriptors);

	if (nullptr != m_callbacks.mapMeshResourcesLoad) {
		m_callbacks.mapMeshResourcesLoad ();
	}

	for (const auto & [libraryName, groupData] : defaultTextures) {
		const PropLibrary & library = * m_propLibraries.at (libraryName);
		const std::map <std::string, PropLibrary::Group> & groups = library.groups ();
		const std::map <std::string, std::shared_ptr <PropMeshResource>> & libraryMeshResources = m_propMeshResources.at (libraryName);

		for (const auto & [groupName, props] : groupData) {
			const PropLibrary::Group & group = groups.at (groupName);

			for (const std::string & propName : props) {
				const PropLibrary::PropMesh & prop = group.meshes.at (propName);

				std::string diffuseFile = libraryMeshResources.at (prop.file)->textureFile;
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

	loadTextureResources (textureDescriptors);

	if (nullptr != m_callbacks.mapTextureResourcesLoad) {
		m_callbacks.mapTextureResourcesLoad ();
	}
}



//  _      ____          _____  ______ _____    _    _ ______ _      _____  ______ _____   _____
// | |    / __ \   /\   |  __ \|  ____|  __ \  | |  | |  ____| |    |  __ \|  ____|  __ \ / ____|
// | |   | |  | | /  \  | |  | | |__  | |__) | | |__| | |__  | |    | |__) | |__  | |__) | (___
// | |   | |  | |/ /\ \ | |  | |  __| |  _  /  |  __  |  __| | |    |  ___/|  __| |  _  / \___ \
// | |___| |__| / ____ \| |__| | |____| | \ \  | |  | | |____| |____| |    | |____| | \ \ ____) |
// |______\____/_/    \_\_____/|______|_|  \_\ |_|  |_|______|______|_|    |______|_|  \_\_____/
//

PropCPUResourceManager::ParsedMeshInfo PropCPUResourceManager::loadMeshResource (const std::string & libraryName, const std::string & meshFile) {
	const std::string meshPath = m_propLibraries.at (libraryName)->path () + "/" + meshFile;

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
	ParsedMeshInfo info {};

	const aiNode * visualNode = scene->mRootNode->mChildren [0];

	{
		std::queue <aiNode *> nodes;
		nodes.push (scene->mRootNode);

		while (false == nodes.empty ()) {
			aiNode * node = nodes.front ();
			nodes.pop ();

			if (node != visualNode) { // TODO: read occluders
				aiMatrix4x4 transform = node->mTransformation;
				// for (aiNode * p = node->mParent; scene->mRootNode != p; p = p->mParent) {
				// 	transform = p->mTransformation * transform;
				// }

				if (true == m_parseCollisionPrimitives) {
					std::string nodeName = node->mName.C_Str ();
					std::ranges::transform (nodeName, nodeName.begin (), [] (char c) -> char {
						return static_cast <char> (std::tolower (c));
					});

					if (true == nodeName.starts_with ("plane")) {
						const aiMesh * rectMesh = scene->mMeshes [node->mMeshes [0]];

						const aiVector3D * _v1 = rectMesh->mVertices;
						const aiVector3D * _v2 = rectMesh->mVertices + 1;
						const aiVector3D * _v3 = rectMesh->mVertices + 2;

						using VertexAndOppositeEdgeLengthSqaurePair = std::pair <const aiVector3D *, double>;

						std::array <VertexAndOppositeEdgeLengthSqaurePair, 3> vedata = {
							VertexAndOppositeEdgeLengthSqaurePair {_v1, (* _v2 - * _v3).SquareLength ()},
							VertexAndOppositeEdgeLengthSqaurePair {_v2, (* _v1 - * _v3).SquareLength ()},
							VertexAndOppositeEdgeLengthSqaurePair {_v3, (* _v1 - * _v2).SquareLength ()}
						};

						auto hIt = std::ranges::max_element (vedata, [] (const VertexAndOppositeEdgeLengthSqaurePair & ved1, const VertexAndOppositeEdgeLengthSqaurePair & ved2) -> bool {
							return ved1.second < ved2.second;
						});

						std::ranges::iter_swap (vedata.begin (), hIt);

						aiVector3D v1 = transform * * vedata [0].first;
						aiVector3D v2 = transform * * vedata [1].first;
						aiVector3D v3 = transform * * vedata [2].first;
						aiVector3D v4 = v2 + v3 - v1;

						info.collider.rectColliders.push_back ({
							.v1 = { .x = v1.x, .y = v1.y, .z = v1.z },
							.v2 = { .x = v2.x, .y = v2.y, .z = v2.z },
							.v3 = { .x = v3.x, .y = v3.y, .z = v3.z },
							.v4 = { .x = v4.x, .y = v4.y, .z = v4.z },
						});
					}
					else if (true == nodeName.starts_with ("box")) {
						const aiMesh * boxMesh = scene->mMeshes [node->mMeshes [0]];
						Collider::VertexType minX, maxX, minY, maxY, minZ, maxZ;
						const aiVector3D v1 = transform * boxMesh->mVertices [0];
						minX = maxX = v1.x;
						minY = maxY = v1.y;
						minZ = maxZ = v1.z;

						for (unsigned int i = 1; i < boxMesh->mNumVertices; i++) {
							const aiVector3D v = transform * boxMesh->mVertices [i];
							if (v.x < minX) {
								minX = v.x;
							}
							else if (v.x > maxX) {
								maxX = v.x;
							}
							if (v.y < minY) {
								minY = v.y;
							}
							else if (v.y > maxY) {
								maxY = v.y;
							}
							if (v.z < minZ) {
								minZ = v.z;
							}
							else if (v.z > maxZ) {
								maxZ = v.z;
							}
						}

						info.collider.boxColliders.push_back ({
							.vMin = { .x = minX, .y = minY, .z = minZ},
							.vMax = { .x = maxX, .y = maxY, .z = maxZ},
						});
					}
					else if (true == nodeName.starts_with ("tri")) {
						const aiMesh * triangleMesh = scene->mMeshes [node->mMeshes [0]];
						aiVector3D v1 = transform * triangleMesh->mVertices [0];
						aiVector3D v2 = transform * triangleMesh->mVertices [1];
						aiVector3D v3 = transform * triangleMesh->mVertices [2];

						info.collider.triangleColliders.push_back ({
							.v1 = { .x = v1.x, .y = v1.y, .z = v1.z },
							.v2 = { .x = v2.x, .y = v2.y, .z = v2.z },
							.v3 = { .x = v3.x, .y = v3.y, .z = v3.z },
						});
					}
				}
			}

			for (int i = 0; i < node->mNumChildren; i++) {
				nodes.push (node->mChildren [i]);
			}
		}
	}


	for (unsigned int meshI = 0; meshI < visualNode->mNumMeshes; meshI++) {
		const aiMesh * mesh = scene->mMeshes [visualNode->mMeshes [meshI]];
		PropMeshResource resource {};

		aiString diffuseMapUrl;
		scene->mMaterials [mesh->mMaterialIndex]->GetTexture (aiTextureType_DIFFUSE, 0, & diffuseMapUrl);

		if (false == diffuseMapUrl.Empty ()) {
			std::string matName (diffuseMapUrl.C_Str ());
			std::ranges::transform (matName, matName.begin (), [] (char c) -> char {
				return static_cast <char> (std::tolower (c));
			});

			resource.textureFile = matName;
		}

		resource.vertexBuffer.resize (mesh->mNumVertices * 3UL);

		for (unsigned i = 0; i < mesh->mNumVertices; i++) {
			resource.vertexBuffer [3 * i + 0] = mesh->mVertices [i].x;
			resource.vertexBuffer [3 * i + 1] = mesh->mVertices [i].y;
			resource.vertexBuffer [3 * i + 2] = mesh->mVertices [i].z;
		}

		resource.indexBuffer.resize (mesh->mNumFaces * 3UL);

		for (unsigned i = 0; i < mesh->mNumFaces; ++i) {
			resource.indexBuffer [3 * i + 0] = static_cast <PropMeshResource::IndexType> (mesh->mFaces [i].mIndices [0]);
			resource.indexBuffer [3 * i + 1] = static_cast <PropMeshResource::IndexType> (mesh->mFaces [i].mIndices [1]);
			resource.indexBuffer [3 * i + 2] = static_cast <PropMeshResource::IndexType> (mesh->mFaces [i].mIndices [2]);
		}

		if (true == mesh->HasTextureCoords (0)) {
			resource.uvBuffer.resize (mesh->mNumVertices * 2UL);

			for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
				resource.uvBuffer [2 * i + 0] = mesh->mTextureCoords [0] [i].x;
				resource.uvBuffer [2 * i + 1] = mesh->mTextureCoords [0] [i].y;
			}
		}

		if (true == mesh->HasNormals ()) {
			resource.normalBuffer.resize (mesh->mNumVertices * 3UL);

			for (unsigned i = 0; i < mesh->mNumVertices; i++) {
				resource.normalBuffer [3 * i + 0] = mesh->mNormals [i].x;
				resource.normalBuffer [3 * i + 1] = mesh->mNormals [i].y;
				resource.normalBuffer [3 * i + 2] = mesh->mNormals [i].z;
			}
		}

		info.meshResources.push_back (std::move (resource));
	}

	// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic,readability-math-missing-parentheses)

	return info;
}

PropCPUResourceManager::PropTextureResource PropCPUResourceManager::loadTextureResource (const std::string & libraryName, const std::string & diffuseFile, const std::string & alphaFile) {
	const std::string diffusePath = m_propLibraries.at (libraryName)->path () + "/" + diffuseFile;

	int width = 0;
	int height = 0;
	int channels = 0;
	int desiredChannels = 3;

	if (false == alphaFile.empty ()) {
		desiredChannels = 4;
	}

	unsigned char * pixels = stbi_load (
		diffusePath.c_str (),
		& width,
		& height,
		& channels,
		desiredChannels
	);

	if (false == alphaFile.empty ()) {
		const std::string alphaPath = m_propLibraries.at (libraryName)->path () + "/" + alphaFile;
		int alphaWidth = 0;
		int alphaHeight = 0;
		int alphaChannels = 0;

		unsigned char *alphaPixels = stbi_load (
			alphaPath.c_str (),
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



//   _____ ______ _______ _______ ______ _____   _____
//  / ____|  ____|__   __|__   __|  ____|  __ \ / ____|
// | |  __| |__     | |     | |  | |__  | |__) | (___
// | | |_ |  __|    | |     | |  |  __| |  _  / \___ \
// | |__| | |____   | |     | |  | |____| | \ \ ____) |
//  \_____|______|  |_|     |_|  |______|_|  \_\_____/
//

const std::map <std::string, std::shared_ptr <PropLibrary>> & PropCPUResourceManager::propLibraries () const {
	return m_propLibraries;
}

const std::map <std::string, std::map <std::string, std::shared_ptr <PropCPUResourceManager::PropMeshResource>>> & PropCPUResourceManager::propMeshResources () const {
	return m_propMeshResources;
}

const std::map <std::string, std::map <std::string, std::shared_ptr <PropCPUResourceManager::PropTextureResource>>> & PropCPUResourceManager::propTextureResources () const {
	return m_propTextureResources;
}

const std::map <std::string, std::map <std::string, std::shared_ptr <PropCPUResourceManager::Collider>>> & PropCPUResourceManager::colliders () const {
	return m_colliders;
}

const PropCPUResourceManager::PropMeshResource & PropCPUResourceManager::getMeshResource (const std::string & libraryName, const std::string & groupName, const std::string & propName) const {
	const std::string & meshFile = m_propLibraries.at (libraryName)->groups ().at (groupName).meshes.at (propName).file;
	return * m_propMeshResources.at (libraryName).at (meshFile);
}

const PropCPUResourceManager::PropTextureResource & PropCPUResourceManager::getTextureResource (const std::string & libraryName, const std::string & groupName, const std::string & propMeshName, const std::string & textureName) const {
	const PropLibrary & library = * m_propLibraries.at (libraryName);

	if (false == textureName.empty ()) {
		const std::string & textureFile = library.groups ().at (groupName).meshes.at (propMeshName).textures.at (textureName);
		return * m_propTextureResources.at (libraryName).at (library.getActualTextureFileName (textureFile));
	}
	else {
		const std::string & meshFile = m_propLibraries.at (libraryName)->groups ().at (groupName).meshes.at (propMeshName).file;
		return * m_propTextureResources.at (libraryName).at (library.getActualTextureFileName (m_propMeshResources.at (libraryName).at (meshFile)->textureFile));
	}
}

const PropCPUResourceManager::PropTextureResource & PropCPUResourceManager::getTextureResource (const std::string & libraryName, const std::string & groupName, const std::string & propSpriteName) const {
	const PropLibrary & library = * m_propLibraries.at (libraryName);

	return * m_propTextureResources.at (libraryName).at (library.getActualTextureFileName (library.groups ().at (groupName).sprites.at (propSpriteName).diffuseFile));
}




//   _____          _      _      ____          _____ _  __ _____
//  / ____|   /\   | |    | |    |  _ \   /\   / ____| |/ // ____|
// | |       /  \  | |    | |    | |_) | /  \ | |    | ' /| (___
// | |      / /\ \ | |    | |    |  _ < / /\ \| |    |  <  \___ \
// | |____ / ____ \| |____| |____| |_) / ____ \ |____| . \ ____) |
//  \_____/_/    \_\______|______|____/_/    \_\_____|_|\_\_____/
//

void PropCPUResourceManager::setMeshResourceLoadCallback (const MeshResourceLoadCallback & callback) {
	m_callbacks.meshResourceLoad = callback;
}
void PropCPUResourceManager::setTextureResourceLoadCallback (const TextureResourceLoadCallback & callback) {
	m_callbacks.textureResourceLoad = callback;
}

void PropCPUResourceManager::setMapMeshResourcesLoadCallback (const MapResourcesLoadCallback & callback) {
	m_callbacks.mapMeshResourcesLoad = callback;
}

void PropCPUResourceManager::setMapTextureResourcesLoadCallback (const MapResourcesLoadCallback & callback) {
	m_callbacks.mapTextureResourcesLoad = callback;
}

void PropCPUResourceManager::clearCallbacks () {
	m_callbacks = {};
}

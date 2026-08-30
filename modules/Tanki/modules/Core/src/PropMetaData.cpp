# include "MapMaster/Tanki/PropMetaData.hpp"

# include <algorithm>
# include <array>
# include <cctype>
# include <iterator>
# include <queue>
# include <string>
# include <utility>
# include <vector>

# include <assimp/matrix4x4.h>
# include <assimp/mesh.h>
# include <assimp/scene.h>
# include <assimp/types.h>
# include <assimp/vector3.h>

using namespace MapMaster::Tanki;

PropMetaData::Mesh::Collider PropMetaData::Mesh::ParseCollider (const aiScene * scene, const aiNode * visualNode) {
	Collider collider;

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

				collider.rectColliders.push_back ({
					.v1 = { .x = v1.x, .y = v1.y, .z = v1.z },
					.v2 = { .x = v2.x, .y = v2.y, .z = v2.z },
					.v3 = { .x = v3.x, .y = v3.y, .z = v3.z },
					.v4 = { .x = v4.x, .y = v4.y, .z = v4.z },
				});
			}
			else if (true == nodeName.starts_with ("box")) {
				const aiMesh * boxMesh = scene->mMeshes [node->mMeshes [0]];
				typename Collider::VertexType minX, maxX, minY, maxY, minZ, maxZ;
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

				collider.boxColliders.push_back ({
					.vMin = { .x = minX, .y = minY, .z = minZ},
					.vMax = { .x = maxX, .y = maxY, .z = maxZ},
				});
			}
			else if (true == nodeName.starts_with ("tri")) {
				const aiMesh * triangleMesh = scene->mMeshes [node->mMeshes [0]];
				aiVector3D v1 = transform * triangleMesh->mVertices [0];
				aiVector3D v2 = transform * triangleMesh->mVertices [1];
				aiVector3D v3 = transform * triangleMesh->mVertices [2];

				collider.triangleColliders.push_back ({
					.v1 = { .x = v1.x, .y = v1.y, .z = v1.z },
					.v2 = { .x = v2.x, .y = v2.y, .z = v2.z },
					.v3 = { .x = v3.x, .y = v3.y, .z = v3.z },
				});
			}
			else if (true == nodeName.starts_with ("occl")) {
			}
		}

		for (int i = 0; i < node->mNumChildren; i++) {
			nodes.push (node->mChildren [i]);
		}
	}

	return collider;
}

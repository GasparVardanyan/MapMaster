# include "RaylibPropResourceManager.hpp"

# include <algorithm>
# include <condition_variable>
# include <cstddef>
# include <execution>
# include <map>
# include <memory>
# include <mutex>
# include <queue>
# include <stack>
# include <string>
# include <thread>
# include <utility>
# include <vector>

# include <raylib.h>
# include <raymath.h>

# include "Map.hpp"
# include "PropLibrary.hpp"
# include "PropResourceManager.hpp"



void RaylibPropResourceManager::loadLibrary (const std::string & path) {
	std::shared_ptr <PropLibrary> library = std::make_shared <PropLibrary> ();
	library->loadDirectory (path);
	m_resourceManager.addPropLibrary (std::move (library));
}

void RaylibPropResourceManager::loadMapLibraries (const Map & map, const std::string & libraryRootDir) {
	for (const auto & [libraryName, groupData] : map.mapObjects ()) {
		loadLibrary (libraryRootDir + "/" + libraryName);
	}
}




//  _      ____          _____  ______ _____   _____
// | |    / __ \   /\   |  __ \|  ____|  __ \ / ____|
// | |   | |  | | /  \  | |  | | |__  | |__) | (___
// | |   | |  | |/ /\ \ | |  | |  __| |  _  / \___ \
// | |___| |__| / ____ \| |__| | |____| | \ \ ____) |
// |______\____/_/    \_\_____/|______|_|  \_\_____/
//

void RaylibPropResourceManager::loadMapResources (const Map & map) {
	std::queue <std::pair <std::string, std::string>> meshQueue;
	std::vector <std::pair <std::string, std::string>> multiMeshVector;
	std::queue <std::pair <std::string, std::string>> textureQueue;

	bool meshesFinished = false;
	bool texturesFinished = false;

	std::mutex meshResourceMutex;
	std::condition_variable meshResourceNotifier;
	std::mutex textureResourceMutex;
	std::condition_variable textureResourceNotifier;

	m_resourceManager.setMeshResourceLoadCallback ([& meshResourceMutex, & meshQueue, & meshResourceNotifier] (const std::string & libraryName, const std::string & meshFile) -> void {
		{
			std::scoped_lock <std::mutex> meshResourceLock (meshResourceMutex);;
			meshQueue.emplace (libraryName, meshFile);
		}

		meshResourceNotifier.notify_one ();
	});

	m_resourceManager.setMultiMeshResourceLoadCallback ([& multiMeshVector] (const std::string & libraryName, const std::string & meshFile) -> void {
		// only 2-4 props are multimesh in proplibs, so the waiting cost is negligible
		multiMeshVector.emplace_back (libraryName, meshFile);
	});

	m_resourceManager.setTextureResourceLoadCallback ([& textureResourceMutex, & textureQueue, & textureResourceNotifier] (const std::string & libraryName, const std::string & textureFile) -> void {
		{
			std::scoped_lock <std::mutex> textureResourceLock (textureResourceMutex);
			textureQueue.emplace (libraryName, textureFile);
		}

		textureResourceNotifier.notify_one ();
	});

	m_resourceManager.setMapMeshResourcesLoadCallback ([& meshesFinished, & meshResourceMutex, & meshResourceNotifier] () -> void {
		{
			std::scoped_lock <std::mutex> meshResourceLock (meshResourceMutex);
			meshesFinished = true;
		}

		meshResourceNotifier.notify_one ();
	});

	m_resourceManager.setMapTextureResourcesLoadCallback ([& texturesFinished, &textureResourceMutex, & textureResourceNotifier] () -> void {
		{
			std::scoped_lock <std::mutex> textureResourceLock (textureResourceMutex);
			texturesFinished = true;
		}

		textureResourceNotifier.notify_one ();
	});

	std::thread resLoaderThread ([this, & map] () -> void {
		m_resourceManager.loadMapResources (map);
	});

	while (true) {
		std::stack <std::pair <std::string, std::string>> meshesToProcess;
		bool finished = false;

		{
			std::unique_lock <std::mutex> meshResourceLock (meshResourceMutex);
			meshResourceNotifier.wait (meshResourceLock, [& meshQueue, & meshesFinished] () -> bool {
				return false == meshQueue.empty () || true == meshesFinished;
			});

			while (false == meshQueue.empty ()) {
				meshesToProcess.push (meshQueue.front ());
				meshQueue.pop ();
			}

			finished = meshesFinished;
		}

		while (false == meshesToProcess.empty ()) {
			std::pair <std::string, std::string> meshDescriptor = meshesToProcess.top ();
			meshesToProcess.pop ();

			PropResourceManager::PropMeshResource & res = const_cast <PropResourceManager::PropMeshResource &> (
				m_resourceManager.propMeshResources ().at (meshDescriptor.first).at (meshDescriptor.second)
			);

			RaylibMeshResource & meshResource = m_meshResources [meshDescriptor.first] [meshDescriptor.second];
			meshResource = loadMeshResource (res);

			UploadMesh (& meshResource.mesh, false);
			meshResource.model = std::shared_ptr <Model> (new Model (LoadModelFromMesh (meshResource.mesh)), unloadMeshResource);
		}

		if (true == finished) {
			break;
		}
	}

	while (true) {
		std::stack <std::pair <std::string, std::string>> texturesToProcess;
		bool finished = false;

		{
			std::unique_lock <std::mutex> textureResourceLock (textureResourceMutex);
			textureResourceNotifier.wait (textureResourceLock, [& textureQueue, & texturesFinished] () -> bool {
				return false == textureQueue.empty () || true == texturesFinished;
			});

			while (false == textureQueue.empty ()) {
				texturesToProcess.push (textureQueue.front ());
				textureQueue.pop ();
			}

			finished = texturesFinished;
		}

		while (false == texturesToProcess.empty ()) {
			std::pair <std::string, std::string> textureDescriptor = texturesToProcess.top ();
			texturesToProcess.pop ();

			const PropResourceManager::PropTextureResource & res = m_resourceManager.propTextureResources ().at (textureDescriptor.first).at (textureDescriptor.second);

			RaylibTextureResource & textureResource = m_textureResources [textureDescriptor.first] [textureDescriptor.second];
			textureResource = loadTextureResource (res);

			textureResource.texture = std::shared_ptr <Texture2D> (new Texture2D (LoadTextureFromImage (textureResource.image)), unloadTextureResource);

			GenTextureMipmaps (textureResource.texture.get ());
			SetTextureFilter (* textureResource.texture, TEXTURE_FILTER_TRILINEAR);
		}

		if (true == finished) {
			break;
		}
	}

	resLoaderThread.join ();
	m_resourceManager.clearCallbacks ();

	std::sort (std::execution::par_unseq, multiMeshVector.begin (), multiMeshVector.end ());
	multiMeshVector.erase (std::unique (std::execution::par_unseq, multiMeshVector.begin (), multiMeshVector.end ()), multiMeshVector.end ());
	loadMultiMeshResources (multiMeshVector);

	// NOLINTBEGIN(*)
	for (const auto & [libraryName, meshFile] : multiMeshVector) {
		const auto & multiMesh = m_resourceManager.propMultiMeshResources ().at (libraryName).at (meshFile);
		RaylibMultiMeshResource & multiMeshResource = m_multiMeshResources [libraryName] [meshFile];
		multiMeshResource.model = std::make_shared <Model> ();
		Model & model = * multiMeshResource.model;

		model.transform = MatrixIdentity();

		model.meshCount = multiMeshResource.meshes.size ();
		model.materialCount = model.meshCount;

		model.meshes = (Mesh *)RL_CALLOC(model.meshCount, sizeof(Mesh));
		model.materials = (Material *)RL_CALLOC(model.materialCount, sizeof(Material));
		model.meshMaterial = (int *)RL_CALLOC(model.meshCount, sizeof(int));

		for (int i = 0; i < model.meshCount; i++) {
			model.meshes[i] = multiMeshResource.meshes.at (i);
			UploadMesh (& model.meshes[i], false);
			model.meshMaterial[i] = i;
			model.materials[i] = LoadMaterialDefault();
			model.materials [i].maps [MATERIAL_MAP_DIFFUSE].texture = * m_textureResources.at (libraryName).at (multiMesh.meshes [i].textureFile).texture;
		}
	}
	// NOLINTEND(*)

	const std::map <std::string, std::shared_ptr <PropLibrary>> & libraries = m_resourceManager.propLibraries ();
	// cppcheck-suppress shadowFunction
	const std::map <std::string, std::map <std::string, PropResourceManager::PropTextureResource>> & textureResources = m_resourceManager.propTextureResources ();

	std::vector <std::pair <std::string, std::string>> spriteDescriptors;

	for (const auto & [libraryName, groups] : map.mapObjects ()) {
		const PropLibrary & library = * libraries.at (libraryName);
		const std::map <std::string, PropLibrary::Group> & libraryGroups = library.groups ();
		for (const auto & [groupName, props] : groups) {
			const PropLibrary::Group & group = libraryGroups.at (groupName);
			for (const auto & [propName, propInfo] : props) {
				if (true == group.sprites.contains (propName)) {
					const auto & sprite = group.sprites.at (propName);
					std::string textureFile = library.getActualTextureFileName (sprite.diffuseFile);
					spriteDescriptors.emplace_back (libraryName, textureFile);

					const PropResourceManager::PropTextureResource & textureResource = textureResources.at (libraryName).at (textureFile);
					if (false == m_spriteInfos.contains (libraryName) || false == m_spriteInfos.at (libraryName).contains (textureFile)) { // FIXME: use propName since theoretically multiple sprites can use the same file with different origins and scales
						const Vector2 size = {
							.x = static_cast <float> (textureResource.width * sprite.scale),
							.y = static_cast <float> (textureResource.height * sprite.scale),
						};

						m_spriteInfos [libraryName] [textureFile] = {
							.origin = {
								.x = static_cast <float> (sprite.originX * size.x),
								.y = static_cast <float> ((1 - sprite.originY) * size.y),
							},
							.size = size,
						};
					}
				}
			}
		}
	}
}

void RaylibPropResourceManager::loadMapResources_OLD (const Map & map) {
	m_resourceManager.loadMapResources (map);

	const std::map <std::string, std::shared_ptr <PropLibrary>> & libraries = m_resourceManager.propLibraries ();
	// cppcheck-suppress shadowFunction
	const std::map <std::string, std::map <std::string, PropResourceManager::PropTextureResource>> & textureResources = m_resourceManager.propTextureResources ();

	std::vector <std::pair <std::string, std::string>> meshDescriptors;
	std::vector <std::pair <std::string, std::string>> textureDescriptors;
	std::vector <std::pair <std::string, std::string>> spriteDescriptors;

	for (const auto & [libraryName, groups] : map.mapObjects ()) {
		const PropLibrary & library = * libraries.at (libraryName);
		const std::map <std::string, PropResourceManager::PropTextureResource> & libraryTextureResources = textureResources.at (libraryName);
		const std::map <std::string, PropLibrary::Group> & libraryGroups = library.groups ();
		for (const auto & [groupName, props] : groups) {
			const PropLibrary::Group & group = libraryGroups.at (groupName);
			for (const auto & [propName, propInfo] : props) {
				if (true == group.meshes.contains (propName)) {
					std::string meshFile = group.meshes.at (propName).file;
					const PropResourceManager::PropMeshResource & meshResource = const_cast <PropResourceManager::PropMeshResource &> (m_resourceManager.propMeshResources ().at (libraryName).at (meshFile));

					meshDescriptors.emplace_back (libraryName, meshFile);

					for (const auto & prop : propInfo) {
						std::string textureName = prop.textureName;
						std::string textureFile;
						if (true == textureName.empty ()) {
							textureFile = meshResource.textureFile;
						}
						else {
							textureFile = group.meshes.at (propName).textures.at (textureName);
						}

						textureFile = library.getActualTextureFileName (textureFile);

						textureDescriptors.emplace_back (libraryName, textureFile);
					}
				}
				else if (true == group.sprites.contains (propName)) {
					const auto & sprite = group.sprites.at (propName);
					std::string textureFile = library.getActualTextureFileName (sprite.diffuseFile);
					textureDescriptors.emplace_back (libraryName, textureFile);
					spriteDescriptors.emplace_back (libraryName, textureFile);

					const PropResourceManager::PropTextureResource & textureResource = libraryTextureResources.at (textureFile);
					if (false == m_spriteInfos.contains (libraryName) || false == m_spriteInfos.at (libraryName).contains (textureFile)) { // FIXME: use propName since theoretically multiple sprites can use the same file with different origins and scales
						const Vector2 size = {
							.x = static_cast <float> (textureResource.width * sprite.scale),
							.y = static_cast <float> (textureResource.height * sprite.scale),
						};

						m_spriteInfos [libraryName] [textureFile] = {
							.origin = {
								.x = static_cast <float> (sprite.originX * size.x),
								.y = static_cast <float> ((1 - sprite.originY) * size.y),
							},
							.size = size,
						};
					}
				}
			}
		}
	}

	std::sort (std::execution::par_unseq, meshDescriptors.begin (), meshDescriptors.end ());
	meshDescriptors.erase (std::unique (std::execution::par_unseq, meshDescriptors.begin (), meshDescriptors.end ()), meshDescriptors.end ());
	std::sort (std::execution::par_unseq, textureDescriptors.begin (), textureDescriptors.end ());
	textureDescriptors.erase (std::unique (std::execution::par_unseq, textureDescriptors.begin (), textureDescriptors.end ()), textureDescriptors.end ());

	loadMeshResources (meshDescriptors);
	loadTextureResources (textureDescriptors);

	for (auto & [libraryName, textureFiles] : m_textureResources) {
		for (auto & [textureFile, textureResource] : textureFiles) {
			textureResource.texture = std::make_shared <Texture2D> (LoadTextureFromImage (textureResource.image));
			GenTextureMipmaps (textureResource.texture.get ());
			SetTextureFilter (* textureResource.texture, TEXTURE_FILTER_TRILINEAR);
		}
	}

	for (auto & [libraryName, meshFiles] : m_meshResources) {
		for (auto & [meshFile, meshResource] : meshFiles) {
			UploadMesh (& meshResource.mesh, false);
			meshResource.model = std::make_shared <Model> (LoadModelFromMesh (meshResource.mesh));
		}
	}
}

void RaylibPropResourceManager::loadMeshResources (const std::vector <std::pair <std::string, std::string>> & meshDescriptors) {
	std::vector <RaylibMeshResource> resources;
	resources.resize (meshDescriptors.size ());

	std::transform (
		std::execution::par_unseq,
		meshDescriptors.cbegin (),
		meshDescriptors.cend (),
		resources.begin (),
		[this] (const std::pair <std::string, std::string> & descriptor) {
			// NOLINTNEXTLINE(hicpp-use-auto,modernize-use-auto)
			PropResourceManager::PropMeshResource & res = const_cast <PropResourceManager::PropMeshResource &> (m_resourceManager.propMeshResources ().at (descriptor.first).at (descriptor.second));
			return loadMeshResource (res);
		}
	);

	std::size_t mI = 0;

	for (const std::pair <std::string, std::string> & descriptor : meshDescriptors) {
		// NOLINTNEXTLINE(hicpp-move-const-arg,performance-move-const-arg)
		m_meshResources [descriptor.first] [descriptor.second] = std::move (resources [mI]);
		mI++;
	}
}

void RaylibPropResourceManager::loadMultiMeshResources (const std::vector <std::pair <std::string, std::string>> & multiMeshDescriptors) {
	std::vector <RaylibMultiMeshResource> resources;
	resources.resize (multiMeshDescriptors.size ());

	std::transform (
		std::execution::par_unseq,
		multiMeshDescriptors.cbegin (),
		multiMeshDescriptors.cend (),
		resources.begin (),
		[this] (const std::pair <std::string, std::string> & descriptor) {
			// NOLINTNEXTLINE(hicpp-use-auto,modernize-use-auto)
			PropResourceManager::PropMultiMeshResource & res = const_cast <PropResourceManager::PropMultiMeshResource &> (m_resourceManager.propMultiMeshResources ().at (descriptor.first).at (descriptor.second));
			return loadMultiMeshResource (res);
		}
	);

	std::size_t mI = 0;

	for (const std::pair <std::string, std::string> & descriptor : multiMeshDescriptors) {
		// NOLINTNEXTLINE(hicpp-move-const-arg,performance-move-const-arg)
		m_multiMeshResources [descriptor.first] [descriptor.second] = std::move (resources [mI]);
		mI++;
	}
}

void RaylibPropResourceManager::loadTextureResources (const std::vector <std::pair <std::string, std::string>> & textureDescriptors) {
	std::vector <RaylibTextureResource> resources;
	resources.resize (textureDescriptors.size ());

	std::transform (
		std::execution::par_unseq,
		textureDescriptors.cbegin (),
		textureDescriptors.cend (),
		resources.begin (),
		[this] (const std::pair <std::string, std::string> & descriptor) {
			const PropResourceManager::PropTextureResource & res = m_resourceManager.propTextureResources ().at (descriptor.first).at (descriptor.second);
			return loadTextureResource (res);
		}
	);

	std::size_t tI = 0;
	for (const std::pair <std::string, std::string> & descriptor : textureDescriptors) {
		// NOLINTNEXTLINE(hicpp-move-const-arg,performance-move-const-arg)
		m_textureResources [descriptor.first] [descriptor.second] = std::move (resources [tI]);
		tI++;
	}
}



//  _      ____          _____  ______ _____    _    _ ______ _      _____  ______ _____   _____
// | |    / __ \   /\   |  __ \|  ____|  __ \  | |  | |  ____| |    |  __ \|  ____|  __ \ / ____|
// | |   | |  | | /  \  | |  | | |__  | |__) | | |__| | |__  | |    | |__) | |__  | |__) | (___
// | |   | |  | |/ /\ \ | |  | |  __| |  _  /  |  __  |  __| | |    |  ___/|  __| |  _  / \___ \
// | |___| |__| / ____ \| |__| | |____| | \ \  | |  | | |____| |____| |    | |____| | \ \ ____) |
// |______\____/_/    \_\_____/|______|_|  \_\ |_|  |_|______|______|_|    |______|_|  \_\_____/
//

// cppcheck-suppress functionStatic
RaylibPropResourceManager::RaylibMeshResource RaylibPropResourceManager::loadMeshResource (PropResourceManager::PropMeshResource & meshResource) {
	RaylibMeshResource m = {};

	m.mesh.vertices = meshResource.vertexBuffer.data ();
	m.mesh.vertexCount = static_cast <int> (meshResource.vertexBuffer.size () / 3);
	if (false == meshResource.uvBuffer.empty ()) {
		m.mesh.texcoords = meshResource.uvBuffer.data ();
	}
	if (false == meshResource.normalBuffer.empty ()) {
		m.mesh.normals = meshResource.normalBuffer.data ();
	}

	m.mesh.triangleCount = static_cast <int> (meshResource.indexBuffer.size () / 3);

	m.mesh.indices = meshResource.indexBuffer.data ();

	return m;
}

RaylibPropResourceManager::RaylibMultiMeshResource RaylibPropResourceManager::loadMultiMeshResource (PropResourceManager::PropMultiMeshResource & meshMultiResource) {
	RaylibMultiMeshResource mm = {};

	for (PropResourceManager::PropMeshResource & meshResource : meshMultiResource.meshes) {
		Mesh mesh {};

		mesh.vertices = meshResource.vertexBuffer.data ();
		mesh.vertexCount = static_cast <int> (meshResource.vertexBuffer.size () / 3);
		if (false == meshResource.uvBuffer.empty ()) {
			mesh.texcoords = meshResource.uvBuffer.data ();
		}
		if (false == meshResource.normalBuffer.empty ()) {
			mesh.normals = meshResource.normalBuffer.data ();
		}

		mesh.triangleCount = static_cast <int> (meshResource.indexBuffer.size () / 3);

		mesh.indices = meshResource.indexBuffer.data ();

		// NOLINTNEXTLINE(hicpp-move-const-arg,performance-move-const-arg)
		mm.meshes.push_back (std::move (mesh));
	}

	return mm;
}

// cppcheck-suppress functionStatic
RaylibPropResourceManager::RaylibTextureResource RaylibPropResourceManager::loadTextureResource (const PropResourceManager::PropTextureResource & textureResource) {
	int pixelFormat = PIXELFORMAT_UNCOMPRESSED_R8G8B8;

	if (4 == textureResource.channels) {
		pixelFormat = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
	}

	Image image = {
		.data = static_cast <void *> (textureResource.pixBuffer.get ()),
		.width = textureResource.width,
		.height = textureResource.height,
		.mipmaps = 1,
		.format = pixelFormat
	};

	return {
		.image = image,
		.texture = {}
	};
}

void RaylibPropResourceManager::unloadMeshResource (Model * model) {
	for (int i = 0; i < model->meshCount; i++) {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		Mesh & m = model->meshes [i];
		m.vertices = nullptr;
		m.vertexCount = 0;
		m.texcoords = nullptr;
		m.normals = nullptr;

		m.triangleCount = 0;

		m.indices = nullptr;
	}
	UnloadModel (* model);
	// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
	delete model;
}

void RaylibPropResourceManager::unloadTextureResource (Texture2D * texture) {
	UnloadTexture (* texture);
	// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
	delete texture;
}



//   _____ ______ _______ _______ ______ _____   _____
//  / ____|  ____|__   __|__   __|  ____|  __ \ / ____|
// | |  __| |__     | |     | |  | |__  | |__) | (___
// | | |_ |  __|    | |     | |  |  __| |  _  / \___ \
// | |__| | |____   | |     | |  | |____| | \ \ ____) |
//  \_____|______|  |_|     |_|  |______|_|  \_\_____/
//

const PropResourceManager & RaylibPropResourceManager::resourceManager () {
	return m_resourceManager;
}

const std::map <std::string, std::map <std::string, RaylibPropResourceManager::RaylibMeshResource>> & RaylibPropResourceManager::meshResources () const {
	return m_meshResources;
}

const std::map <std::string, std::map <std::string, RaylibPropResourceManager::RaylibMultiMeshResource>> & RaylibPropResourceManager::multiMeshResources () const {
	return m_multiMeshResources;
}

const std::map <std::string, std::map <std::string, RaylibPropResourceManager::RaylibTextureResource>> & RaylibPropResourceManager::textureResources () const {
	return m_textureResources;
}

const std::map <std::string, std::map <std::string, RaylibPropResourceManager::RaylibSpriteInfo>> & RaylibPropResourceManager::spriteInfos () const {
	return m_spriteInfos;
}

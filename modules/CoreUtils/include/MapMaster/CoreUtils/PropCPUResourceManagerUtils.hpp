# pragma once

# include <concepts>
# include <cstdio>
# include <memory>
# include <string>

# include <assimp/Importer.hpp>
# include <assimp/scene.h>
# include <stb_image.h>

namespace MapMaster::CoreUtils {

namespace PropCPUResourceManagerUtils {

struct DummyTexture;
struct DummyMesh;

struct Texture {
	// NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members)
	const int width;
	const int height;
	const int channels;
	const std::shared_ptr <unsigned char> pixels;
	const bool valid;
	// NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)

private:
	struct FileDeleter {
		void operator() (std::FILE * file) const {
			if (nullptr != file) {
				(void) std::fclose (file); // NOLINT(cppcoreguidelines-owning-memory)
			}
		}
	};

public:
	using FileUniquePtr = std::unique_ptr <std::FILE, FileDeleter>;

	using Dummy = DummyTexture;

	Texture (int width, int height, int channels, unsigned char * pixels, bool valid = true)
		: width (width), height (height), channels (channels), pixels (pixels), valid (valid)
	{}

	Texture (int width, int height, int channels, unsigned char * pixels, std::invocable <unsigned char *> auto deleter, bool valid = true)
		: width (width), height (height), channels (channels), pixels (pixels, deleter), valid (valid)
	{}

	static Texture FromSTBI (int width, int height, int channels, unsigned char * pixels) {
		return {width, height, channels, pixels, stbi_image_free, true};
	}

	static Texture FromFileHandles (FileUniquePtr diffuseFileHandle, FileUniquePtr alphaFileHandle);
};

struct Mesh {
	std::shared_ptr <Assimp::Importer> importer;
	const aiScene * scene = nullptr;
	bool valid = false;

	using Dummy = DummyMesh;

	explicit Mesh (const std::string & path, int assimpPostProcessorSteps, unsigned int assimpImporterRemoveComponentFlags);

protected:
	Mesh () = default;
};

struct DummyTexture : Texture {
	DummyTexture ()
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
		: Texture (1, 1, 4, new unsigned char [4] {0xFF, 0x00, 0x00, 0x80}, false)
	{}
};

struct DummyMesh : Mesh {
	DummyMesh (int assimpPostProcessorSteps, unsigned int assimpImporterRemoveComponentFlags);
};

} // namespace PropCPUResourceManagerUtils

} // namespace MapMaster::CoreUtils

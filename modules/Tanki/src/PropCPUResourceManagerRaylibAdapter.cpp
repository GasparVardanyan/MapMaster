# pragma once

# include <cstddef>
# include <cstdlib>
# include <cstring>
# include <cwchar>
# include <memory>

# include "MapMaster/Tanki/PropCPUResourceManagerRaylibAdapter.hpp"

using namespace MapMaster::Tanki;



PropCPUResourceManagerRaylibAdapter::PropTextureResource PropCPUResourceManagerRaylibAdapter::PropTextureResource::clone () {
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

# include "PropRaylibResourceManager.hpp"

# include <string>
# include <utility>

# include "Map.hpp"
# include "PropLibrary.hpp"
# include "PropResourceManager.hpp"

void PropRaylibResourceManager::loadLibrary (const std::string & path) {
	PropLibrary library;
	library.loadDirectory (path);
	m_resourceManager.addPropLibrary (std::move (library));
}

PropResourceManager & PropRaylibResourceManager::resourceManager () {
	return m_resourceManager;
}

void PropRaylibResourceManager::loadMapLibraries (const Map & map, const std::string & libraryRootDir) {
	for (const auto & [libraryName, groups] : map.mapObjects ()) {
		loadLibrary (libraryRootDir + "/" + libraryName);
	}
}

# pragma once

# include <string>

# include "Map.hpp"
# include "PropResourceManager.hpp"

class PropRaylibResourceManager {
public:
	PropResourceManager & resourceManager ();
	void loadLibrary (const std::string & path);
	void loadMapLibraries (const Map & map, const std::string & libraryRootDir);

private:
	PropResourceManager m_resourceManager;
	std::string m_libraryRootDir;
};

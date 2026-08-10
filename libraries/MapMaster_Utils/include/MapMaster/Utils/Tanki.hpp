# pragma once

# include <map>
# include <memory>
# include <string>
# include <vector>

# include <raylib.h>

namespace MapMaster::Tanki {
class Map;
class PropLibrary;
class PropResourceManager;
class RaylibMap;
} // namespace MapMaster::Tanki



namespace MapMaster::Utils::Tanki {



using PropLibraryNameToPathVectorMap = std::map <
	std::string,
	std::vector <std::string>
>;

using PropLibraryNameToLibraryVectorMap = std::map <
	std::string,
	std::vector <std::shared_ptr <MapMaster::Tanki::PropLibrary>>
>;

using PropLibraryNameToResourceManagerVectorMap = std::map <
	std::string,
	std::vector <std::shared_ptr <MapMaster::Tanki::PropResourceManager>>
>;



namespace Window {
void OpenRaylibWindow (int width, int height, const std::string & title = {}, int logLevel = LOG_NONE);
void DrawRaylibMapInCurrentWindow (std::shared_ptr <MapMaster::Tanki::RaylibMap> rmap, float scale = 1.0, const std::string & msg1 = {}, const std::string & msg2 = {});
void CloseRaylibWindow ();
} // namespace Window



std::shared_ptr <MapMaster::Tanki::RaylibMap> LoadRaylibMap (const std::string & libraryRootPath, const std::string & mapFile, float scale, bool haveCanonicalLibraryStructure = false);


PropLibraryNameToPathVectorMap FindPropLibraryPaths (const std::string & libraryRootPath);
PropLibraryNameToLibraryVectorMap LoadPropLibraries (const PropLibraryNameToPathVectorMap & nameToPathVectorMap);
PropLibraryNameToResourceManagerVectorMap LoadPropLibraryResources (const PropLibraryNameToLibraryVectorMap & nameToLibraryVectorMap);
std::shared_ptr <MapMaster::Tanki::PropResourceManager> LoadPropLibraryResources (std::shared_ptr <MapMaster::Tanki::PropLibrary> library);

std::vector <std::string> FindMapLibraryPaths (const PropLibraryNameToPathVectorMap & propLibraries, const MapMaster::Tanki::Map & map);
std::vector <std::string> FindMapLibraryNames (const MapMaster::Tanki::Map & map);



}  // namespace MapMaster::Utils::Tanki

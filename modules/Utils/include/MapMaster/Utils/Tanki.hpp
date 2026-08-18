# pragma once

# include <raylib.h>
# include <map>
# include <memory>
# include <string>
# include <vector>

namespace MapMaster::Tanki {
class Map;
class PropLibrary;
template <class>
class MapRenderer;
} // namespace MapMaster::Tanki



namespace MapMaster::Utils::Tanki {



// using PropLibraryNameToPathVectorMap = std::map <
// 	std::string,
// 	std::vector <std::string>
// >;
//
// using PropLibraryNameToLibraryVectorMap = std::map <
// 	std::string,
// 	std::vector <std::shared_ptr <MapMaster::Tanki::PropLibrary>>
// >;
//
// using PropLibraryNameToResourceManagerVectorMap = std::map <
// 	std::string,
// 	std::vector <std::shared_ptr <MapMaster::Tanki::PropCPUResourceManager>>
// >;



namespace Window {
template <class MapRendererBackend>
void OpenMapWindow (int width, int height, const std::string & title = {}, int logLevel = LOG_NONE);
template <class MapRendererBackend>
void DrawMapRendererInCurrentWindow (std::shared_ptr <MapMaster::Tanki::MapRenderer <MapRendererBackend>> rmap, float scale = 1.0, const std::string & msg1 = {}, const std::string & msg2 = {});
template <class MapRendererBackend>
void CloseMapWindow ();
} // namespace Window



template <class MapRendererBackend>
std::shared_ptr <MapMaster::Tanki::MapRenderer <MapRendererBackend>> LoadMapRenderer (const std::string & libraryRootPath, const std::string & mapFile, float scale, bool haveCanonicalLibraryStructure = false);


// PropLibraryNameToPathVectorMap FindPropLibraryPaths (const std::string & libraryRootPath);
// PropLibraryNameToLibraryVectorMap LoadPropLibraries (const PropLibraryNameToPathVectorMap & nameToPathVectorMap);
// PropLibraryNameToResourceManagerVectorMap LoadPropLibraryResources (const PropLibraryNameToLibraryVectorMap & nameToLibraryVectorMap);
// std::shared_ptr <MapMaster::Tanki::PropCPUResourceManager> LoadPropLibraryResources (std::shared_ptr <MapMaster::Tanki::PropLibrary> library);
//
// std::vector <std::string> FindMapLibraryPaths (const PropLibraryNameToPathVectorMap & propLibraries, const MapMaster::Tanki::Map & map);
// std::vector <std::string> FindMapLibraryNames (const MapMaster::Tanki::Map & map);

}  // namespace MapMaster::Utils::Tanki

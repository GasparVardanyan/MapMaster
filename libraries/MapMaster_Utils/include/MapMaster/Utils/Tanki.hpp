# pragma once

# include <map>
# include <string>
# include <vector>

# include <MapMaster/Tanki/Map.hpp>


namespace MapMaster::Utils {

struct Tanki {
	static std::map <std::string, std::vector <std::string>> FindPropLibraries (const std::string & libraryRootPath);
	static std::vector <std::string> FindMapLibraries (const std::map <std::string, std::vector <std::string>> & propLibraries, const MapMaster::Tanki::Map & map);
};

}  // namespace MapMaster::Tanki

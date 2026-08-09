# include "MapMaster/Utils/Tanki.hpp"

# include <exception>
# include <iostream>
# include <map>
# include <string>
# include <vector>
# include <filesystem>

# include <pugixml.hpp>

using namespace MapMaster::Tanki;
using namespace MapMaster::Utils;

std::map <std::string, std::vector <std::string>> Tanki::FindPropLibraries (const std::string & libraryRootPath) {
	std::map <std::string, std::vector <std::string>> libraries;

	for (const std::filesystem::directory_entry & diEnt : std::filesystem::recursive_directory_iterator (libraryRootPath)) {
		if (true == diEnt.is_regular_file ()) {
			const std::filesystem::path & diEntPath = diEnt.path ();
			std::string diEntName = diEntPath.filename ().string ();

			if ("library.xml" == diEntName) {
				pugi::xml_document libXml;
				pugi::xml_parse_result pr = libXml.load_file (diEntPath.c_str ());

				if (true || pugi::xml_parse_status::status_ok == pr) {
					std::string libraryName = libXml.child ("library").attribute ("name").value ();

					libraries [libraryName].push_back (diEntPath.parent_path ().string ());
				}
				else {
					std::cerr << "Utils: PropLibrary failed to parse " << diEntPath << ". " << pr.description () << ".\n";
				}
			}
		}
	}

	return libraries;
}

std::vector <std::string> Tanki::FindMapLibraries (const std::map <std::string, std::vector <std::string>> & propLibraries, const Map & map) {
	std::vector <std::string> mapLibraries;

	for (const auto & [libraryName, groupData] : map.mapObjects ()) {
		if (true == propLibraries.contains (libraryName)) {
			const std::vector <std::string> & librarySources = propLibraries.at (libraryName);

			if (1 == librarySources.size ()) {
				mapLibraries.push_back (librarySources.back ());
			}
			if (1 < librarySources.size ()) {
				std::cerr << "WARN: " << librarySources.size () << " sources found for " << libraryName << '\n';
				for (const auto & x : librarySources) {
					std::cout << "\tsource: " << x << '\n';
				}
				// std::terminate ();
			}
		}
		else {
			std::cerr << "FAIL: no sources found for " << libraryName << '\n';
		}
	}

	return mapLibraries;
}

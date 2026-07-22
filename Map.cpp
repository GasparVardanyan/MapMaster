# include "Map.hpp"

# include <iostream>
# include <map>
# include <string>
# include <vector>

# include "PropLibrary.hpp"

void Map::loadFile (const std::string & path) {
	pugi::xml_document mapXml;
	pugi::xml_parse_result parseResult = mapXml.load_file (path.c_str ());

	if (pugi::xml_parse_status::status_ok != parseResult.status) {
		std::cout << "Map failed to parse " << path << ". " << parseResult.description () << ".\n";
		return;
	}

	parse (mapXml);
}

void Map::parse (pugi::xml_node mapXml) {
	const pugi::xml_node map = mapXml.child ("map");
	const std::string mapVersion = map.attribute ("version").value ();

	if ("1.0.Light" == mapVersion) {
		const pugi::xml_node staticGeometry = map.child ("static-geometry");
		for (const pugi::xml_node prop : staticGeometry.children ("prop")) {
			const std::string libraryName = prop.attribute ("library-name").value ();
			const std::string groupName = prop.attribute ("group-name").value ();
			const std::string propName = prop.attribute ("name").value ();

			m_mapObjects [libraryName] [groupName] [propName].push_back ({
				.rotationZ = prop.child ("rotation").child ("z").text ().as_double (),
				.positionX = prop.child ("position").child ("x").text ().as_double (),
				.positionY = prop.child ("position").child ("y").text ().as_double (),
				.positionZ = prop.child ("position").child ("z").text ().as_double (),
				.textureName = prop.child ("texture-name").text ().as_string ()
			});
		}
	}
}

const std::map <
		std::string,
		std::map <
			std::string,
			std::map <
				std::string,
				std::vector <Map::MapObject>
			>
		>
> & Map::mapObjects () const {
	return m_mapObjects;
}

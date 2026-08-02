# include "MapMaster/Tanki/Map.hpp"

# include <initializer_list>
# include <iostream>
# include <map>
# include <string>

# include <pugixml.hpp>

void Map::loadFile (const std::string & path) {
	pugi::xml_document mapXml;
	pugi::xml_parse_result parseResult = mapXml.load_file (path.c_str ());

	if (pugi::xml_parse_status::status_ok != parseResult.status) {
		std::cerr << "Map failed to parse " << path << ". " << parseResult.description () << ".\n";
		return;
	}

	parse (mapXml.child ("map"));
}

void Map::parse (pugi::xml_node map) {
	const std::string mapVersion = map.attribute ("version").value ();

	// FIXME: implement the other versions
	if (true || "1.0.Light" == mapVersion) {
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

const Map::MapObjectCollection & Map::mapObjects () const {
	return m_mapObjects;
}

void Map::clear () {
	m_mapObjects = {};
}

# pragma once

# include <map>
# include <string>
# include <type_traits>
# include <vector>

namespace pugi { class xml_node; }  // namespace pugi

namespace MapMaster::Tanki {

// cppcheck-suppress-begin unusedStructMember
class Map {
public:
	struct MapObject {
		using RotationType = double;
		using PositionType = double;

		static_assert (std::is_floating_point_v <RotationType>);
		static_assert (std::is_floating_point_v <PositionType>);

		RotationType rotationZ = 0;
		PositionType positionX = 0;
		PositionType positionY = 0;
		PositionType positionZ = 0;

		std::string textureName;
	};

	/**
	 * @brief mapObjects {libraryName => groupName => propName => {mapObjects}}
	 */
	using MapObjectCollection = std::map <
		std::string,
		std::map <
			std::string,
			std::map <
				std::string,
				std::vector <MapObject>
			>
		>
	>;

public:
	void loadFile (const std::string & path);
	void parse (pugi::xml_node mapXml);
	void clear ();

	[[nodiscard]] const MapObjectCollection & mapObjects () const;

private:
	MapObjectCollection m_mapObjects;
};
// cppcheck-suppress-end unusedStructMember

}  // namespace MapMaster::Tanki

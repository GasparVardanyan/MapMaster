# pragma once

# include <map>
# include <string>
# include <type_traits>
# include <vector>

namespace pugi { class xml_node; }  // namespace pugi

// cppcheck-suppress-begin unusedStructMember
class Map {
public:
	struct MapObject {
		using RotationType = double;
		static_assert (std::is_floating_point_v <RotationType>);

		using PositionType = double;
		static_assert (std::is_floating_point_v <PositionType>);

		RotationType rotationZ = 0;
		PositionType positionX = 0;
		PositionType positionY = 0;
		PositionType positionZ = 0;

		std::string textureName;
	};

public:
	void loadFile (const std::string & path);
	void parse (pugi::xml_node mapXml);

	[[nodiscard]] const std::map <
		std::string,
		std::map <
			std::string,
			std::map <
				std::string,
				std::vector<MapObject>
			>
		>
	> & mapObjects () const;

private:
	std::map <
		std::string,
		std::map <
			std::string,
			std::map <
				std::string,
				std::vector<MapObject>
			>
		>
	> m_mapObjects;
};
// cppcheck-suppress-end unusedStructMember

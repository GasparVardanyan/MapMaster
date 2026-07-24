# pragma once // NOLINT(portability-avoid-pragma-once)

# include <concepts>
# include <map>
# include <string>
# include <type_traits>

namespace pugi {
class xml_document; class xml_node;
}  // namespace pugi

// cppcheck-suppress-begin unusedStructMember
class PropLibrary {
public:
	struct PropMesh {
		std::string file;
		std::map <std::string, std::string> textures;
	};

	struct PropSprite {
		using OriginType = double;
		static_assert (std::is_floating_point_v <OriginType>);

		using ScaleType = double;
		static_assert (std::is_floating_point_v <ScaleType>);

		template <std::floating_point> struct OriginXDefault;
		template <std::floating_point> struct OriginYDefault;
		template <std::floating_point> struct ScaleDefault;

		template <>
		struct OriginXDefault <float> { static constexpr float value = 0.5F; };
		template <>
		struct OriginXDefault <double> { static constexpr double value = 0.5; };
		template <>
		// NOLINTNEXTLINE(google-runtime-float)
		struct OriginXDefault <long double> { static constexpr double value = 0.5L; };

		template <>
		struct OriginYDefault <float> { static constexpr float value = 0.5F; };
		template <>
		struct OriginYDefault <double> { static constexpr double value = 0.5; };
		template <>
		// NOLINTNEXTLINE(google-runtime-float)
		struct OriginYDefault <long double> { static constexpr double value = 0.5L; };

		template <>
		struct ScaleDefault <float> { static constexpr float value = 1.0F; };
		template <>
		struct ScaleDefault <double> { static constexpr double value = 1.0; };
		template <>
		// NOLINTNEXTLINE(google-runtime-float)
		struct ScaleDefault <long double> { static constexpr double value = 1.0L; };

		OriginType originX = OriginXDefault <OriginType>::value;
		OriginType originY = OriginYDefault <OriginType>::value;
		ScaleType scale = ScaleDefault <ScaleType>::value;

		std::string diffuseFile;
	};

	struct Group {
		std::map <std::string, PropMesh> meshes;
		std::map <std::string, PropSprite> sprites;
	};

public:
	void loadDirectory (const std::string & path);
	void clear ();

	[[nodiscard]] std::string actualTextureFile (const std::string & oldFile) const;
	[[nodiscard]] const std::string & name () const;
	[[nodiscard]] const std::map <std::string, std::string> & diffuseMap () const;
	[[nodiscard]] const std::map <std::string, std::string> & opacityMap () const;
	[[nodiscard]] const std::map <std::string, Group> & groups () const;
	[[nodiscard]] const std::string & path () const;

private:
	void parse (pugi::xml_node libXml, const pugi::xml_document & imgXml);
	void parseImageMap (const pugi::xml_document & imgXml);
	void parseLibrary (pugi::xml_node libXml);
	void parseGroup (pugi::xml_node groupXml);
	void parsePropMesh (pugi::xml_node meshXml, PropMesh & mesh);
	void parsePropSprite (pugi::xml_node spriteXml, PropSprite & sprite);

private:
	struct {
		std::string libraryName;
		std::map <std::string, std::string> diffuseMap;
		std::map <std::string, std::string> opacityMap;
		std::map <std::string, Group> groups;
	} m_xmlData;
	std::string m_path;
};
// cppcheck-suppress-end unusedStructMember

# include "PropLibrary.hpp"

# include <algorithm>
# include <cctype>
# include <cstring>
# include <exception>
# include <iostream>
# include <map>
# include <string>
# include <type_traits>

# include <pugixml.hpp>

void PropLibrary::loadDirectory (const std::string & path) {
	m_path = path;

	const std::string & libXmlPath = path + "/library.xml";
	const std::string & imgXmlPath = path + "/images.xml";

	pugi::xml_document libXml, imgXml;

	pugi::xml_parse_result parseResult = libXml.load_file (libXmlPath.c_str ());

	if (pugi::xml_parse_status::status_ok != parseResult.status) {
		std::cerr << "PropLibrary failed to parse " << libXmlPath << ". " << parseResult.description () << ".\n";
		return;
	}

	parseResult = imgXml.load_file (imgXmlPath.c_str ());

	parse (libXml.child ("library"), imgXml);
}

void PropLibrary::parse (pugi::xml_node libXml, const pugi::xml_document & imgXml) {
	if (false == imgXml.children ().empty ()) {
		parseImageMap (imgXml);
	}

	parseLibrary (libXml);
}

void PropLibrary::parseImageMap (const pugi::xml_document & imgXml) {
	for (const pugi::xml_node image : imgXml.child ("images").children ()) {
		const std::string originalName = image.attribute ("name").value ();
		std::string diffuseName = image.attribute ("new-name").value ();
		std::string opacityName = image.attribute ("opacity").value ();

		std::ranges::transform (diffuseName, diffuseName.begin (), [] (char c) -> char {
			return static_cast <char> (std::tolower (c));
		});

		m_xmlData.diffuseMap.insert ({originalName, diffuseName});

		if (false == opacityName.empty ()) {
			std::ranges::transform (opacityName, opacityName.begin (), [] (char c) -> char {
				return static_cast <char> (std::tolower (c));
			});

			m_xmlData.opacityMap.insert ({originalName, opacityName});
		}
	}
}

void PropLibrary::parseLibrary (pugi::xml_node libXml) {
	m_xmlData.libraryName = libXml.attribute ("name").value ();

	for (const pugi::xml_node group : libXml.children ("prop-group")) {
		parseGroup (group);
	}
}

void PropLibrary::parseGroup (pugi::xml_node groupXml) {
	Group & group = m_xmlData.groups [groupXml.attribute ("name").value ()];

	for (const pugi::xml_node prop : groupXml.children ("prop")) {
		const pugi::xml_node propData = prop.first_child ();

		if (0 == std::strcmp ("mesh", propData.name ())) {
			parsePropMesh (
				propData,
				group.meshes [prop.attribute ("name").value ()]
			);
		}
		else if (0 == std::strcmp ("sprite", propData.name ())) {
			parsePropSprite (
				propData,
				group.sprites [prop.attribute ("name").value ()]
			);
		}
		else {
			std::cerr << "Unknown prop type " << propData.name () << '\n';
			std::cerr << "group: " << groupXml.attribute ("name").value () << '\n';
			std::cerr << "prop: " << prop.attribute ("name").value () << '\n';
			std::cerr << path () << '\n';
			std::terminate ();
		}
	}
}

// cppcheck-suppress functionStatic
void PropLibrary::parsePropMesh (pugi::xml_node meshXml, PropMesh & mesh) {
	mesh.file = meshXml.attribute ("file").value ();
	std::ranges::transform (mesh.file, mesh.file.begin (), [] (char c) -> char {
		return static_cast <char> (std::tolower (c));
	});

	for (const pugi::xml_node textureXml : meshXml.children ("texture")) {
		mesh.textures [textureXml.attribute ("name").value ()] = textureXml.attribute ("diffuse-map").value ();
		std::string & textureDiffuseMap = mesh.textures [textureXml.attribute ("name").value ()];

		std::ranges::transform (textureDiffuseMap, textureDiffuseMap.begin (), [] (char c) -> char {
			return static_cast <char> (std::tolower (c));
		});
	}
}

void PropLibrary::parsePropSprite (pugi::xml_node spriteXml, PropSprite & sprite) {
	sprite.diffuseFile = spriteXml.attribute ("file").value ();

	std::ranges::transform (sprite.diffuseFile, sprite.diffuseFile.begin (), [] (char c) -> char {
		return static_cast <char> (std::tolower (c));
	});

	if constexpr (true == std::is_same_v <PropSprite::OriginType, float>) {
		sprite.originX = spriteXml.attribute ("origin-x").as_float (PropSprite::OriginXDefault <PropSprite::OriginType>::value);
		sprite.originY = spriteXml.attribute ("origin-y").as_float (PropSprite::OriginYDefault <PropSprite::OriginType>::value);
	}
	else {
		sprite.originX = spriteXml.attribute ("origin-x").as_double (PropSprite::OriginXDefault <PropSprite::OriginType>::value);
		sprite.originY = spriteXml.attribute ("origin-y").as_double (PropSprite::OriginYDefault <PropSprite::OriginType>::value);
	}

	if constexpr (true == std::is_same_v <PropSprite::ScaleType, float>) {
		sprite.scale = spriteXml.attribute ("scale").as_float (PropSprite::ScaleDefault <PropSprite::ScaleType>::value);
	}
	else {
		sprite.scale = spriteXml.attribute ("scale").as_double (PropSprite::ScaleDefault <PropSprite::ScaleType>::value);
	}
}

const std::string & PropLibrary::name () const {
	return m_xmlData.libraryName;
}

const std::map <std::string, std::string> & PropLibrary::diffuseMap () const {
	return m_xmlData.diffuseMap;
}

const std::map <std::string, std::string> & PropLibrary::opacityMap () const {
	return m_xmlData.opacityMap;
}

const std::map <std::string, PropLibrary::Group> & PropLibrary::groups () const {
	return m_xmlData.groups;
}

const std::string & PropLibrary::path () const {
	return m_path;
}

std::string PropLibrary::actualTextureFile (const std::string & oldFile) const {
	if (true == m_xmlData.diffuseMap.contains (oldFile)) {
		return m_xmlData.diffuseMap.at (oldFile);
	}
	else {
		return oldFile;
	}
}

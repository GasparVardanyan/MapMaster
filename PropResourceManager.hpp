# pragma once

# include <map>
# include <string>
# include <type_traits>
# include <vector>

# include "PropLibrary.hpp"

// cppcheck-suppress-begin unusedStructMember
class PropResourceManager {
public:
	struct PropMeshResource {
		using VertexType = float;
		static_assert (std::is_floating_point_v <VertexType>);
		using NormalType = float;
		static_assert (std::is_floating_point_v <NormalType>);
		using TexCoordType = float;
		static_assert (std::is_floating_point_v <TexCoordType>);
		using IndexType = unsigned short; // NOLINT(google-runtime-int)
		static_assert (std::is_integral_v <IndexType>);

		std::string textureFile;
		std::vector <VertexType> vertexBuffer;
		std::vector <NormalType> normalBuffer;
		std::vector <TexCoordType> uvBuffer;
		std::vector <IndexType> indexBuffer;
	};

	struct PropSpriteResource {};

	struct Group {
		std::map <std::string, PropMeshResource> meshResources;
		std::map <std::string, PropSpriteResource> spriteResources;
	};

public:
	void addPropLibrary (const PropLibrary & propLibrary);
	void addPropLibrary (PropLibrary && propLibrary);

	void loadResources (const std::string & libraryName, const std::string & groupName, const std::string & propName);

	const std::map <std::string, PropLibrary> & propLibraries () const;
	const std::map <std::string, std::map <std::string, Group>> & propResources () const;

private:
	std::map <std::string, PropLibrary> m_propLibraries;
	std::map <std::string, std::map <std::string, Group>> m_propResources;
};
// cppcheck-suppress-end unusedStructMember

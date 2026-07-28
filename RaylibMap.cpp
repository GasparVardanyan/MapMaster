# include "RaylibMap.hpp"
# include "RaylibPropResourceManager.hpp"
# include <memory>

std::shared_ptr <const RaylibPropResourceManager> RaylibMap::resourceManager () {
	return std::shared_ptr <const RaylibPropResourceManager> (m_raylibResourceManager);
}

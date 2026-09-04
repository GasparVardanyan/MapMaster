# include "MapMaster/Tanki/PrimitiveFactory.hpp"
# include "MapMaster/Tanki/PropMetaData.hpp"

template <class PrimitiveFactoryBackend>
std::shared_ptr <typename MapMaster::Tanki::PrimitiveFactory <PrimitiveFactoryBackend>::SpriteMeshResource> MapMaster::Tanki::PrimitiveFactory <PrimitiveFactoryBackend>::GetSharedSpriteMeshResource (const PropMetaData::Sprite & meta) {
	return {};
}

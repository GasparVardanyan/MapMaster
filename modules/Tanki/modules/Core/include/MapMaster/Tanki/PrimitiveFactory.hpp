# pragma once

# include <type_traits>

namespace MapMaster::Tanki {

template <class PrimitiveFactoryBackend, typename = void>
struct IsPrimitiveFactoryBackend : std::false_type {};

template <class PrimitiveFactoryBackend>
struct IsPrimitiveFactoryBackend <
	PrimitiveFactoryBackend,
	std::void_t <
		typename PrimitiveFactoryBackend::SpriteResource,
		void
	>
> : std::true_type {};

template <class PrimitiveFactoryBackend>
class PrimitiveFactory {
public:
	using Backend = std::enable_if_t <
		IsPrimitiveFactoryBackend <PrimitiveFactoryBackend>::value,
		PrimitiveFactoryBackend
	>;

	using SpriteMeshResource = Backend::SpriteMeshResource;

private:
};

}  // namespace MapMaster::Tanki

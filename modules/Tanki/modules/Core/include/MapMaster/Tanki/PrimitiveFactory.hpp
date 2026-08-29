# pragma once

# include <type_traits>

namespace MapMaster::Tanki {

template <class PrimitiveFactoryBackend>
struct IsPrimitiveFactoryBackend : std::false_type {};

template <class PrimitiveFactoryBackend>
class PrimitiveFactory {
public:
	using Backend = std::enable_if_t <
		IsPrimitiveFactoryBackend <PrimitiveFactoryBackend>::value,
		PrimitiveFactoryBackend
	>;
};

}  // namespace MapMaster::Tanki

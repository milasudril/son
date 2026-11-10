#ifndef SON_INT_TYPES_HPP
#define SON_INT_TYPES_HPP

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <climits>

namespace son
{
	template<size_t N>
	struct make_int_type;

	template<>
	struct make_int_type<8>
	{ using type = int8_t; };

	template<>
	struct make_int_type<16>
	{ using type = int16_t; };

	template<>
	struct make_int_type<32>
	{ using type = int32_t; };

	template<>
	struct make_int_type<64>
	{ using type = int64_t; };

	template<size_t N>
	using int_t = make_int_type<N>::type;

	template<size_t N>
	using uint_t = std::make_unsigned_t<int_t<N>>;

	static_assert(CHAR_BIT == 8);

	using half_intptr_t = int_t<4*sizeof(void*)>;
	using half_uintptr_t = uint_t<4*sizeof(void*)>;
}

#endif

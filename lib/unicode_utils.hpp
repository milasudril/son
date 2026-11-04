#ifndef SON_UNICODE_UTILS_HPP
#define SON_UNICODE_UTILS_HPP

#include "./exception.hpp"

#include <cstdint>
#include <array>
#include <expected>

namespace son
{
	class codepoint_factory_utf16
	{
	public:
		enum class result
		{
			more_data_needed,
			low_surrogate_as_first_code_unit,
			high_surrogate_as_second_code_unit,
			second_code_unit_is_not_a_low_surrogate
		};

		static constexpr bool is_high_surrogate(char16_t value)
		{ return value >= 0xD800 && value <= 0xDBFF; }

		static constexpr bool is_low_surrogate(char16_t value)
		{ return value >= 0xDC00 && value <= 0xDFFF; }

		static constexpr char32_t combine_surrogate_pair(std::array<char16_t, 2> pair)
		{
			auto const high = std::get<0>(pair);
			auto const low = std::get<1>(pair);
			return static_cast<char32_t>(0x10000 + ((high - 0xD800) << 10) + (low - 0xDC00));
		}

		constexpr void reset()
		{ *this = codepoint_factory_utf16{}; }

		constexpr std::expected<char32_t, result> make_codepoint(char16_t value)
		{
			switch(m_cu_count)
			{
				case 0:
					if(is_low_surrogate(value))
					{ return std::unexpected{result::low_surrogate_as_first_code_unit}; }

					if(!is_high_surrogate(value))
					{ return static_cast<char32_t>(value); }

					std::get<0>(m_cus) = value;
					++m_cu_count;
					return std::unexpected{result::more_data_needed};

				case 1:
					if(!requires_additional_code_unit())
					{ raise_internal_error("Unexpected second code unit"); }

					if(is_high_surrogate(value))
					{ return std::unexpected{result::high_surrogate_as_second_code_unit}; }

					if(!is_low_surrogate(value))
					{ return std::unexpected{result::second_code_unit_is_not_a_low_surrogate}; }

					std::get<1>(m_cus) = value;
					return combine_surrogate_pair(m_cus);

				default:
					raise_internal_error("Too many elements in UTF-16 codepoint");
			}
		}

		constexpr bool requires_additional_code_unit() const
		{
			auto const first_cu = m_cus.front();
			return m_cu_count == 1 && is_high_surrogate(first_cu);
		}

	private:
		uint16_t m_cu_count{};
		std::array<char16_t, 2> m_cus{};
	};
};

#endif

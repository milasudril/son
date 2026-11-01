#ifndef SON_UNICODE_UTILS_HPP
#define SON_UNICODE_UTILS_HPP

#include "./exception.hpp"

#include <cstdint>
#include <array>

namespace son
{
	class utf16_codepoint
	{
	public:
		enum class add_codeunit_result
		{
			success,
			low_surrogate_as_first_code_unit,
			high_surrogate_as_second_code_unit,
			second_code_unit_is_not_a_low_surrogate
		};

		static constexpr bool is_high_surrogate(char16_t value)
		{ return value >= 0xD800 && value <= 0xDBFF; }

		static constexpr bool is_low_surrogate(char16_t value)
		{ return value >= 0xDC00 && value <= 0xDFFF; }

		constexpr void reset()
		{ *this = utf16_codepoint{}; }

		constexpr add_codeunit_result add_code_unit(char16_t value)
		{
			switch(m_cu_count)
			{
				case 0:
					if(is_low_surrogate(value))
					{ return add_codeunit_result::low_surrogate_as_first_code_unit; }
					std::get<0>(m_cus) = value;
					++m_cu_count;
					return add_codeunit_result::success;

				case 1:
					if(!requires_additional_code_unit())
					{ raise_internal_error("Unexpected second code unit"); }

					if(is_high_surrogate(value))
					{ return add_codeunit_result::high_surrogate_as_second_code_unit; }

					if(!is_low_surrogate(value))
					{ return add_codeunit_result::second_code_unit_is_not_a_low_surrogate; }

					std::get<1>(m_cus) = value;
					++m_cu_count;
					return add_codeunit_result::success;

				default:
					raise_internal_error("Too many elements in UTF-16 codepoint");
			}
		}

		constexpr bool requires_additional_code_unit() const
		{
			auto const first_cu = m_cus.front();
			return m_cu_count == 1 && is_high_surrogate(first_cu);
		}

		constexpr char32_t to_codepoint() const
		{
			if(requires_additional_code_unit())
			{ raise_internal_error("Unexpected second code unit"); }

			switch(m_cu_count)
			{
				case 0:
					return char32_t{};

				case 1:
					return static_cast<char32_t>(std::get<0>(m_cus));

				default:
					auto const high = std::get<0>(m_cus);
					auto const low = std::get<1>(m_cus);
					return static_cast<char32_t>(0x10000 + ((high - 0xD800) << 10) + (low - 0xDC00));
			}
		}

	private:
		uint16_t m_cu_count{};
		std::array<char16_t, 2> m_cus{};
	};
};

#endif

#ifndef SON_UNICODE_UTILS_HPP
#define SON_UNICODE_UTILS_HPP

#include "./exception.hpp"

#include <cstdint>
#include <array>

namespace son
{
	/**
	 * \todo:
	 * * Low Surrogates (0xDC00 – 0xDFFF): Cannot appear as the first code unit (prevents isolated low
	 *   surrogates).
	 * * High Surrogates (0xD800 – 0xDBFF): When appearing as the first code unit, they must be followed
	 *   by a valid low surrogate.
	 * * Isolated Surrogates: to_codepoint() validates that a single code unit is not an un-paired
	 *   surrogate before converting.
	 */
	class utf16_codepoint
	{
	public:
		constexpr void add_code_unit(char16_t value)
		{
			if(m_cu_count >= std::size(m_cus))
			{ raise_internal_error("To many elements in UTF-16 codepoint"); }
			// NOLINTNEXTLINE
			m_cus[m_cu_count] = value;
			++m_cu_count;
		}

		constexpr bool requires_additional_code_unit() const
		{
			auto const first_cu = m_cus.front();
			return m_cu_count < 2 && first_cu >= 0xD800 && first_cu <= 0xDBFF;
		}

		constexpr char32_t to_codepoint() const
		{
			switch(m_cu_count)
			{
				case 0:
					return {};

				case 1:
					//NOLINTNEXTLINE
					return static_cast<char32_t>(m_cus[0]);

				default:
					//NOLINTNEXTLINE
					char16_t const high = m_cus[0];
					//NOLINTNEXTLINE
					char16_t const low = m_cus[1];
					return static_cast<char32_t>(0x10000 + ((high - 0xD800) << 10) + (low - 0xDC00));
			}
		}

	private:
		uint16_t m_cu_count{};
		std::array<char16_t, 2> m_cus{};
	};
};

#endif

#ifndef SON_UNICODE_UTILS_HPP
#define SON_UNICODE_UTILS_HPP

#include "lib/core/exception.hpp"
#include "lib/core/scope_utils.hpp"

#include <cstdint>
#include <array>
#include <expected>

/**
 * \defgroup unicode_utils Helpers for encoding/decoding codepoints
 */

namespace son
{
	/**
	 * \brief Decodes a UTF-16 value, or a UTF-16 surrogate pair
	 * \ingroup unicode_utils
	 */
	class utf16_decoder
	{
	public:

		/**
		 * \brief The result of trying to make a codepoint
		 */
		enum class result
		{
			/**
			 * \brief The operation was not completed. Rerun it with another code unit.
			 */
			more_data_needed,

			/**
			 * \brief Error code used when a low surrogate occurs first
			 */
			low_surrogate_as_first_code_unit,

			/**
			 * \brief Error code used when something different than a low surrogate occurs last
			 */
			second_code_unit_is_not_a_low_surrogate
		};

		/**
		 * \brief The lowest code unit for a low surrogate
		 */
		static constexpr auto low_surrogate_min = static_cast<char16_t>(0xDC00);

		/**
		 * \brief The highest code unit for a low surrogate
		 */
		static constexpr auto low_surrogate_max = static_cast<char16_t>(0xDFFF);

		/**
		 * \brief The lowest code unit for a high surrogate
		 */
		static constexpr auto high_surrogate_min = static_cast<char16_t>(0xD800);

		/**
		 * \brief The highest code unit for a high surrogate
		 */
		static constexpr auto high_surrogate_max = static_cast<char16_t>(0xDBFF);


		/**
		 * \brief Test whether or not value is a high surrogate
		 */
		static constexpr bool is_high_surrogate(char16_t value)
		{ return value >= high_surrogate_min && value <= high_surrogate_max; }

		/**
		 * \brief Test whether or not value is a low surrogate
		 */
		static constexpr bool is_low_surrogate(char16_t value)
		{ return value >= low_surrogate_min && value <= low_surrogate_max; }

		/**
		 * \brief Combines the values in pair and returns a codepoint
		 * \warning It is up to the caller to verify that pair is a valid UTF-16 surrogate pair
		 */
		static constexpr char32_t combine_surrogate_pair(std::array<char16_t, 2> pair)
		{
			auto const high = std::get<0>(pair);
			auto const low = std::get<1>(pair);

			return static_cast<char32_t>(
					0x10000
				+ (static_cast<uint32_t>(high - high_surrogate_min) << 10U)
				+ (low - low_surrogate_min));
		}

		/**
		 * \brief Resets the state of the decoder
		 */
		constexpr void reset()
		{ *this = utf16_decoder{}; }

		/**
		 * \brief Tries to convert value into a codepoint
		 * \return The corresponding codepoint, or a result if the operation was not successful
		 */
		constexpr std::expected<char32_t, result> try_to_make_codepoint(char16_t value)
		{
			auto at_exit = make_scope_exit([this]{reset();});
			switch(m_cu_count)
			{
				case 0:
					if(is_low_surrogate(value))
					{ return std::unexpected{result::low_surrogate_as_first_code_unit}; }

					if(!is_high_surrogate(value))
					{ return static_cast<char32_t>(value); }

					std::get<0>(m_cus) = value;
					++m_cu_count;
					at_exit.release();
					return std::unexpected{result::more_data_needed};

				case 1:
					if(!is_low_surrogate(value))
					{ return std::unexpected{result::second_code_unit_is_not_a_low_surrogate}; }

					std::get<1>(m_cus) = value;
					return combine_surrogate_pair(m_cus);

				default:
					raise_internal_error("Too many elements in UTF-16 codepoint");
			}
		}

		/**
		 * \brief Queries whether or not the decoder is in its initial state
		 */
		constexpr bool is_in_initial_state() const
		{ return m_cu_count == 0; }

	private:
		uint16_t m_cu_count{};
		std::array<char16_t, 2> m_cus{};
	};

	/**
	 * \brief Encodes value into buffer as UTF-8
	 * \ingroup unicode_utils
	 * \return A number representing the number of bytes written to buffer, or zero if value
	 *         is outside the valid range
	 */
	template<class OutputBuffer>
	constexpr size_t encode_as_utf8(char32_t value, OutputBuffer& buffer)
	{
		using value_type = OutputBuffer::value_type;
		if (value <= 0x7F)
		{
			buffer.push_back(static_cast<value_type>(value));
			return 1;
		}

		if (value <= 0x7FF)
		{
			buffer.push_back(static_cast<value_type>(0xC0U | (value >> 6U)));
			buffer.push_back(static_cast<value_type>(0x80U | (value & 0x3FU)));
			return 2;
		}

		if (value <= 0xFFFF)
		{
			buffer.push_back(static_cast<value_type>(0xE0U | (value >> 12U)));
			buffer.push_back(static_cast<value_type>(0x80U | ((value >> 6U) & 0x3FU)));
			buffer.push_back(static_cast<value_type>(0x80U | (value & 0x3FU)));
			return 3;
    }

		if (value <= 0x10FFFF)
		{
			buffer.push_back(static_cast<value_type>(0xF0U | (value >> 18U)));
			buffer.push_back(static_cast<value_type>(0x80U | ((value >> 12U) & 0x3FU)));
			buffer.push_back(static_cast<value_type>(0x80U | ((value >> 6U) & 0x3FU)));
			buffer.push_back(static_cast<value_type>(0x80U | (value & 0x3FU)));
			return 4;
		}
		return 0;
	}
};

#endif

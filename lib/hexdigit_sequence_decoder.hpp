#ifndef SON_HEXDIGIT_SEQUENCE_DECODER_HPP
#define SON_HEXDIGIT_SEQUENCE_DECODER_HPP

#include "./scope_utils.hpp"
#include "lib/exception.hpp"

#include <bit>
#include <expected>
#include <climits>

namespace son
{
	enum class hexdigit_sequence_decode_result{
		/**
			* \brief The operation was not completed. Rerun it with another code unit.
			*/
		more_data_needed,

		/**
			* \brief Error code used when the input character is not the EndMarker or a valid hexdigit
			*/
		invalid_input,

		/**
			* \brief Error code used when EndMarker appears at odd index
			*/
		incomplete_byte
	};

	struct bad_digit_tag{};

	constexpr std::expected<std::byte, bad_digit_tag> to_nibble(char digit)
	{
		if(digit >= '0' && digit <= '9')
		{ return static_cast<std::byte>(digit - '0');}

		digit &= static_cast<char>(0b1101'1111);

		if(digit >= 'A' && digit <= 'F')
		{ return static_cast<std::byte>(10 + digit - 'A');}

		return std::unexpected{bad_digit_tag{}};
	}

	template<class TargetType, char EndMarker = '.'>
	class hexdigit_sequence_decoder
	{
	public:
		static_assert(CHAR_BIT == 8);

		using result = hexdigit_sequence_decode_result;

		constexpr void reset()
		{
			*this = hexdigit_sequence_decoder{};
		}

		constexpr std::expected<TargetType, result> try_to_make_value(char digit)
		{
			auto at_exit = make_scope_exit([this]{reset();});
			if(digit == EndMarker)
			{
				if(m_elem_count%8 != 0)
				{ return std::unexpected{result::incomplete_byte}; }

				return std::byteswap(m_value);
			}

			if(m_elem_count < 8*sizeof(TargetType))
			{
				auto const val = to_nibble(digit);
				if(!val.has_value())
				{ return std::unexpected{result::invalid_input}; }
				m_value |= (to_nibble(digit) << m_elem_count);
				m_elem_count += 4;

				at_exit.reset();
			}
			raise_internal_error("Too many digits in hexdigit sequence");
		}

		/**
		 * \brief Queries whether or not the decoder is in its initial state
		 */
		constexpr bool is_in_initial_state() const
		{ return m_elem_count == 0; }

	private:
		TargetType m_value{};
		TargetType m_elem_count{};
	};
}
#endif

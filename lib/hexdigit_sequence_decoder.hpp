#ifndef SON_HEXDIGIT_SEQUENCE_DECODER_HPP
#define SON_HEXDIGIT_SEQUENCE_DECODER_HPP

#include "lib/core/scope_utils.hpp"
#include "lib/core/exception.hpp"

#include <expected>
#include <climits>

namespace son
{
	/**
	 * \defgroup numconv_utils Utilities for conversion between strings and numbers
	 */

	/**
	 * \brief The result of trying to make a value from a hexdigit
	 * \ingroup numconv_utils
	 */
	enum class hexdigit_sequence_decode_result{
		/**
			* \brief The operation was not completed. Rerun it with another code unit.
			*/
		more_data_needed,

		/**
			* \brief Error code used when the input character is not the endmarker or a valid hexdigit
			*/
		invalid_input,

		/**
			* \brief Error code used when Endmarker appears at odd index
			*/
		incomplete_byte
	};

	/**
	 * \brief Tag used to denote that to_nibble failed
	 * \ingroup numconv_utils
	 */
	struct bad_digit_tag{};

	/**
	 * \brief Converts digit into a nibble (case insensitive)
	 * \ingroup numconv_utils
	 */
	constexpr std::expected<std::byte, bad_digit_tag> to_nibble(unsigned char digit)
	{
		if(digit >= '0' && digit <= '9')
		{ return static_cast<std::byte>(digit - '0');}

		digit &= static_cast<unsigned char>(0b1101'1111);

		if(digit >= 'A' && digit <= 'F')
		{ return static_cast<std::byte>(10 + digit - 'A');}

		return std::unexpected{bad_digit_tag{}};
	}

	/**
	 * \brief Decodes a hexdigit sequence into a number
	 * \tparam TargetType The type of the output value such as uint32_t
	 * \tparam Endmarker The symbol used to terminate a sequence that is shorter than `2*sizeof(T)`
	 *                   digigts
	 * \ingroup numconv_utils
	 */
	template<class TargetType, char Endmarker>
	class hexdigit_sequence_decoder
	{
	public:
		static_assert(CHAR_BIT == 8);

		using result = hexdigit_sequence_decode_result;

		static constexpr auto endmarker = Endmarker;

		/**
		 * \brief Resets the state of the decoder
		 */
		constexpr void reset()
		{ *this = hexdigit_sequence_decoder{}; }

		/**
		 * \brief Tries to convert digit into a value
		 * \return The corresponding value, or a result if the operation was not successful
		 */
		constexpr std::expected<TargetType, result> try_to_make_value(char digit)
		{
			auto at_exit = make_scope_exit([this]{reset();});
			if(digit == endmarker)
			{ return m_value; }

			if(m_elem_count < 8*sizeof(TargetType))
			{
				auto const val = to_nibble(static_cast<unsigned char>(digit));
				if(!val.has_value())
				{ return std::unexpected{result::invalid_input}; }

				m_value = (m_value << 4U) | static_cast<TargetType>(*val);
				m_elem_count += 4;
				if(m_elem_count == 8*sizeof(TargetType))
				{ return m_value; }

				at_exit.release();
				return std::unexpected{result::more_data_needed};
			}
			raise_internal_error("Too many digits in hexdigit sequence");
		}

		/**
		 * \brief Queries whether or not the decoder is in its initial state
		 */
		constexpr bool is_in_initial_state() const
		{ return m_elem_count == 0 && m_elem_count == 0; }

	private:
		TargetType m_value{};
		TargetType m_elem_count{};
	};
}
#endif

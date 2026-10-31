#ifndef SON_PARENSTRING_UTILS_HPP
#define SON_PARENSTRING_UTILS_HPP

#include "./exception.hpp"

#include <optional>

namespace son
{
	/**
	 * \defgroup parenstring Functionality related to strings in parenthesis
	 */

	/**
	 * \brief The result of a decoding a parenstring
	 * \ingroup parenstring
	 */
	enum class parenstring_decode_result{
		/**
		 * \brief The operation was completed without errors
		 */
		done,
		/**
		 * \brief A character in the input buffer was not accepted
		 */
		illegal_char_in_string,
		/**
		 * \brief An invalid escape sequence appeared in the input buffer
		 */
		more_data_needed
	};

	/**
	 * \brief Requirements on a class that can manage the state during the decoding of a string
	 * \ingroup parenstring
	 */
	template<class T>
	concept parenstring_decoder = requires(T& decoder, char ch_in)
	{
		/**
		 * \brief The character used to start an escape sequence, typically `\`
		 */
		{T::string_escape_char} -> std::same_as<char const&>;

		/**
		 * \brief The character used to start a parenstring, typically `(`
		 */
		{T::parenstring_begin_char} -> std::same_as<char const&>;

		/**
		 * \brief The character used to end a parenstring, typically `)`
		 */
		{T::parenstring_end_char} -> std::same_as<char const&>;

		/**
		 * \brief Should store `ch_in` in an internal buffer, if it is accepted
		 * \return true if `ch_in` was accepted, otherwise `false`
		 */
		{decoder.accept_char(ch_in)} -> std::same_as<bool>;

		/**
		 * \brief Should increment the parenthesis nesting count
		 */
		{decoder.inc_paren_count()} -> std::same_as<void>;


		/**
		 * \brief Should decrement the parenthesis nesting count
		 * \returns 0 when the last parenthesis was closed
		 */
		{decoder.dec_paren_count()}-> std::same_as<size_t>;

		/**
		 * \brief Updates the state of the decoder
		 */
		{decoder.set_state(std::declval<typename T::state>())} -> std::same_as<void>;

		/**
		 * \brief Returns the state of the decoder
		 */
		{decoder.get_state()} -> std::same_as<typename T::state>;

		/** \name stringdec_state
		 *  The different state decoder can be in
		 */
		///@{
		{T::state::string_escape} -> std::same_as<typename T::state>;
		{T::state::string_normal} -> std::same_as<typename T::state>;
		///@}
	};

	/**
	 * \brief Handles the `normal` state when decoding a parenstring
	 *
	 * When
	 *
	 * * `ch_in` equals StringDecoder::string_escape_char (as described in parenstring_decoder), the
	 *   decoder state is updated to `string_escape`, and no result is returned
	 *
	 * * `ch_in` equals StringDecoder::parenstring_end_char (as described in parenstring_decoder), the
	 *   the parenthesis count is incremented, and the function returns nothing
	 *
	 * * `ch_in` equals StringDecoder::parenstring_end_char (as described in parenstring_decoder), and
	 *
	 *   * the last parenthesis was closed, the state is kept, and the function returns
	 *    parenstring_decode_result::done
	 *
	 *   * the last parenthesis was nto closed, the state is kept, and nothing is returned
	 *
	 * * In any other case, decoder is asked to `accept_char` (as described in parenstring_decoder).
	 *   If decoder accepts `ch_in`, no result is returned, otherwise the return value is
	 *   parenstring_decode_result::illegal_char_in_string. In any case, the decoder state is not
	 *   updated.
	 *
	 * \ingroup parenstring
	 */
	template<parenstring_decoder StringDecoder>
	std::optional<parenstring_decode_result> decode_parenstring_state_normal(
		StringDecoder& decoder,
		char ch_in
	)
	{
		using decoder_state = StringDecoder::state;
		switch(ch_in)
		{
			case StringDecoder::string_escape_char:
				decoder.set_state(decoder_state::string_escape);
				return std::nullopt;

			case StringDecoder::parenstring_begin_char:
				decoder.inc_paren_count();
				return std::nullopt;

			case StringDecoder::parenstring_end_char:
				if(decoder.dec_paren_count() == 0)
				{ return parenstring_decode_result::done; }

				return std::nullopt;

			default:
				if(!decoder.accept_char(ch_in))
				{ return parenstring_decode_result::illegal_char_in_string; }

				return std::nullopt;
		}
	}

	/**
	 * \brief Handles the `normal` state when decoding a parenstring
	 *
	 * When
	 *
	 * * `ch_in` equals StringDecoder::parenstring_begin_char (as described in parenstring_decoder)
	 *    or equals StringDecoder::parenstring_end_char (as described in parenstring_decoder), the
	 *    decoder is asked to accept the character. If it does, the decoder state is updated to
	 *    `string_normal`, and the function returns nothing. Otherwise, the state is kept and the
	 *    function returns parenstring_decode_result::illegal_char_in_string.
	 *
	 * * In any other case the following happens in sequence
	 *
	 *   1. decoder is asked to accept the escpae char. If it doesn't, the function returns
	 *      parenstring_decode_result::illegal_char_in_string
	 *
	 *   2. decoder is asked to accept `ch_in`. If it doesn't, the function returns
	 *      parenstring_decode_result::illegal_char_in_string
	 *
	 *   3. the state is updated to decoder_state::string_normal, and the function returns nothing
	 *
	 * \ingroup parenstring
	 */
	template<parenstring_decoder StringDecoder>
	constexpr std::optional<parenstring_decode_result> decode_parenstring_state_string_escape(
		StringDecoder& decoder,
		char ch_in
	)
	{
		using decoder_state = StringDecoder::state;
		switch(ch_in)
		{
			case StringDecoder::parenstring_begin_char:
			case StringDecoder::parenstring_end_char:
				if(!decoder.accept_char(ch_in))
				{ return parenstring_decode_result::illegal_char_in_string; }

				decoder.set_state(decoder_state::string_normal);
				return std::nullopt;

			default:
				if(!decoder.accept_char(StringDecoder::string_escape_char))
				{ return parenstring_decode_result::illegal_char_in_string; }

				if(!decoder.accept_char(ch_in))
				{ return parenstring_decode_result::illegal_char_in_string; }

				decoder.set_state(decoder_state::string_normal);
				return std::nullopt;
		}
	}

	/**
	 * \brief Decodes a parenstring from an input range managed by the decoder
	 *
	 * Iterates through the input range provided by the decoder, processing characters
	 * according to the current state (`string_normal` or `string_escape`). It coordinates
	 * state transitions, handles escape characters, manages nested parentheses, and commits
	 * consumed characters back to the decoder's input range.
	 *
	 * \tparam StringDecoder A type satisfying the `parenstring_decoder` concept.
	 * \param decoder Reference to the decoder state and input range manager.
	 * \return A `parenstring_decode_result` indicating whether decoding completed successfully
	 *         (`done`), encountered an illegal character (`illegal_char_in_string`), or
	 *         requires more data (`more_data_needed`).
	 *
	 * The state machine for decoding a string is presented below
	 *
	 * @startuml
	 * [*] --> string_normal
	 *
	 * state string_normal {
	 * }
	 *
	 * state string_escape {
	 * }
	 *
	 * string_normal --> string_escape : string_escape_char
	 * string_normal --> string_normal : parenstring_begin_char
	 * string_normal --> string_normal : parenstring_end_char (count > 0)
	 * string_normal --> [*] : parenstring_end_char (count == 0)
	 * string_normal --> string_normal : ch_in (accepted)
	 * string_normal --> [*] : ch_in (rejected)
	 *
	 * string_escape --> string_normal : parenstring_begin/end_char (accepted)
	 * string_escape --> Error : parenstring_begin/end_char (rejected)
	 * string_escape --> string_normal : ch_in (accepted)
	 * string_escape --> Error : ch_in (rejected)
	 * @enduml
	 *
	 * \ingroup parenstring
	 */
	template<parenstring_decoder StringDecoder>
	parenstring_decode_result decode_parenstring(StringDecoder& decoder)
	{
		auto src_range = decoder.get_input_range();
		using src_range_type = decltype(src_range);
		auto first = src_range.begin();
		auto const last = src_range.end();

		auto commit = [&](auto status){
			if(status == parenstring_decode_result::done)
			{ ++first; }
			decoder.set_input_range(src_range_type{first, last});
			return status;
		};

		while(first != last)
		{
			auto const ch_in = *first;
			using decoder_state = StringDecoder::state;
// This function does not know how to handle every possible decoder_state.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
			switch(decoder.get_state())
			{
				case decoder_state::string_normal:
					if(auto const result = decode_parenstring_state_normal(decoder, ch_in))
					{ return commit(*result); }
					break;

				case decoder_state::string_escape:
					if(auto const result = decode_parenstring_state_string_escape(decoder, ch_in))
					{ return commit(*result); }
					break;

				default:
					raise_internal_error("Unexpected return value from get_state");
			}
#pragma GCC diagnostic pop
			++first;
		}
		return commit(parenstring_decode_result::more_data_needed);
	}
}
#endif

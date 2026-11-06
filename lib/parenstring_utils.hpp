#ifndef SON_PARENSTRING_UTILS_HPP
#define SON_PARENSTRING_UTILS_HPP

#include "./exception.hpp"

#include <optional>

namespace son
{
	/**
	 * \defgroup parenstring Functionality related to strings in parenthesis
	 *
	 * A **parenstring** is a string encoding format designed to minimize the need for escape
	 * sequences. This is achieved by using matched parentheses as delimiters. The assumption is that
	 * most text will have matching pairs of parentheses. If not, an escape character can be used to
	 * write the mismatched parenthesis
	 *
	 * * A begin character (typically `(`) increments the parenthesis nesting level
	 *
	 * * An end character (typically `)`) decrements the nesting level
	 *
	 * * The parenstring terminates when an end character reduces the nesting count to zero
	 *
	 * To encode the begin and end character, when it would result in mismatched begin/end characters,
	 * a corresponding escape sequence is used. Typically <code>\\</code> is used as the escape
	 * character, and `-` is used as escape code for the escape character itself. Using these symbols
	 * the following escape sequences are recognised
	 *
	 * | Sequences     | Interpretation |
	 * |---------------|----------------|
	 * | `\(` and `\)` | The delimiter `(` or `)`, is treated as a literal character |
	 * | `\-`          | A single <code>\\</code> |
	 * | General escape sequences (`\X`) | The sequence is treated literally |
	 *
	 * That is, the escape character is only treated as such when it starts an recognised escape
	 * sequence.
	 *
	 * A use-case for mismatched parentheses is a half-open interval. Using
	 * `(`, `)` and <code>\\</code>, it could be written as
	 *
	 * ```
	 * I = \(0, x(a + b)]
	 * ```
	 *
	 * The representation is not unique. The above could also be written as
	 *
	 * ```
	 * I = (0, x\(a + b)]
	 * ```
	 *
	 * An encoder may also take the simple approach to escape all parentheses. To optimise the
	 * encoding, it needs to run a parentheses matching step. If it finds to many `(`, the
	 * excess `(` are escaped. Likewise, if it finds too many `)`, the excess `)` are escaped.
	 *
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
		 * \brief An invalid escape sequence appeared in the input buffer
		 */
		more_data_needed,

		/**
		 * \brief A character in the input buffer was not accepted
		 */
		illegal_char_in_string,
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
		 * \brief The character used for the escape character escape code, typically `-`
		 */
		{T::parenstring_escape_char_escape_code} -> std::same_as<char const&>;

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
	 * * `ch_in` equals StringDecoder::parenstring_end_char (as described in parenstring_decoder),
	 *   decoder is asked to accept the character. If it doesn't
	 *   parenstring_decode_result::illegal_char_in_string is returned. Otherwise, the parenthesis
	 *   count is incremented, and the function returns nothing. In any case the state is kept.
	 *
	 * * `ch_in` equals StringDecoder::parenstring_end_char (as described in parenstring_decoder), and
	 *
	 *   * the last parenthesis was closed, the state is kept, and the function returns
	 *    parenstring_decode_result::done
	 *
	 *   * the last parenthesis was not closed, decoder is asked to accept the character. If it
	 *     doesn't, parenstring_decode_result::illegal_char_in_string is returned. Otherwise nothing
	 *     is returned. In any case the state is kept.
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
				if(!decoder.accept_char(ch_in))
				{ return parenstring_decode_result::illegal_char_in_string; }

				decoder.inc_paren_count();
				return std::nullopt;

			case StringDecoder::parenstring_end_char:
				if(decoder.dec_paren_count() == 0)
				{ return parenstring_decode_result::done; }

				if(!decoder.accept_char(ch_in))
				{ return parenstring_decode_result::illegal_char_in_string; }

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
	 * * `ch_in` equals StringDecoder::parenstring_escape_char_escape_code (as described in
	 *    parenstring_decoder), the decoder is asked to
	 *    accept StringDecoder::string_escape_char (as described in parenstring_decoder). If it does,
	 *    the decoder state is updated to `string_normal`, and the function returns nothing. Otherwise
	 *    the state is kept and the function returns
	 *    parenstring_decode_result::illegal_char_in_string.
	 *
	 * * In any other case the following happens in sequence
	 *
	 *   1. decoder is asked to accept the escape char. If it doesn't, the function returns
	 *      parenstring_decode_result::illegal_char_in_string.
	 *
	 *   2. decoder is asked to accept `ch_in`. If it doesn't, the function returns
	 *      parenstring_decode_result::illegal_char_in_string.
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

			case StringDecoder::parenstring_escape_char_escape_code:
				if(!decoder.accept_char(StringDecoder::string_escape_char))
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
	 * \brief Decodes a parenstring from the input range managed by the decoder.
	 *
	 * This function iterates through the input range provided by \p decoder, dispatching
	 * character processing based on the current decoder state (`string_normal` or `string_escape`).
	 * It manages input range advancement and commits results upon completion or error.
	 *
	 * \tparam StringDecoder A type satisfying the \ref parenstring_decoder concept.
	 * \param decoder Reference to the stateful decoder instance.
	 * \return A parenstring_decode_result
	 *         - parenstring_decode_result::done if the parenstring terminated successfully.
	 *         - parenstring_decode_result::illegal_char_in_string if an invalid character or
	 *           unexpected sequence was encountered.
	 *         - parenstring_decode_result::more_data_needed if the input range was exhausted before
	 *           the parenstring terminated.
	 *
	 * \ingroup parenstring
	 *
	 * The following diagram illustrates the states and transition logic executed by decode_parenstring:
	 *
	 * @startuml
	 * skinparam DefaultTextAlignment center
	 * hide empty description
	 *
	 * [*] --> string_normal : start decoding
	 *
	 * string_normal --> string_escape : ch_in == string_escape_char
	 *
	 * string_normal --> [*] : ch_in == parenstring_end_char\n && dec_paren_count() == 0
	 *
	 * string_normal --> [*] : error
	 *
	 * string_normal --> string_normal : Everything else
	 *
	 * string_escape --> string_normal
	 *
	 * string_escape --> [*] : error
	 *
	 * @enduml
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

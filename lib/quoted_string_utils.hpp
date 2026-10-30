#ifndef SON_STRINGCONV_HPP
#define SON_STRINGCONV_HPP

#include "./exception.hpp"

#include <optional>

/**
 * \defgroup stringconv String conversion utilities
 * \brief Contains classes and functions for decoding and encoding strings
 */

namespace son
{
	/**
	 * \brief The result of a decoding a quoted string
	 * \ingroup stringconv
	 */
	enum class quoted_string_decode_result
	{
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
		invalid_escape_sequence,

		/**
		 * \brief An invalid escape sequence appeared in the input buffer
		 */
		more_data_needed,

		/**
		 * \brief An invalid UTF-16 surrogate pair appeared in the input buffer
		 */
		illformed_escaped_utf16_surrogate_pair
	};

	/**
	 * \brief The result of decoding an escape sequence representing a Unicode codepoint
	 * \ingroup stringconv
	 */
	enum class accept_codepoint_hexdigit_result
	{
		/**
		 * \brief The operation was completed without errors
		 */
		done,

		/**
		 * \brief The decoded escape sequence resulted in an illegal character
		 */
		illegal_char_in_string,

		/**
		 * \brief A character in the input buffer was not accepted (not an hexdigit)
		 */
		invalid_input,

		/**
		 * \brief A surrogate pair was detected and a second escape sequence is required
		 */
		second_surrogate_element_required
	};

	/**
	 * \brief Requirements on a class that can manage the state during the decoding of a string
	 * \ingroup stringconv
	 */
	template<class T>
	concept quoted_string_decoder = requires(T& decoder, char ch_in)
	{
		/**
		 * \brief The character used to start an escape sequence, typically `\`
		 */
		{T::string_escape_char} -> std::same_as<char const&>;

		/**
		 * \brief The character used to end a string, typically `"`
		 */
		{T::quoted_string_end_char} -> std::same_as<char const&>;

		/**
		 * \brief The character used to start a codepoint, typically `u`
		 */
		{T::codepoint_begin_char} -> std::same_as<char const&>;

		/**
		 * \brief Should store `ch_in` in an internal buffer, if it is accepted
		 * \return true if `ch_in` was accepted, otherwise `false`
		 */
		{decoder.accept_char(ch_in)} -> std::same_as<bool>;

		/**
		 * \brief Should map `ch_in` to the character that corresponds to this particular escape
		 *        code, and store the result in an internal buffer
		 *
		 * \return true if `ch_in` was accepted, otherwise `false`
		 */
		{decoder.accept_escape_code(ch_in)} -> std::same_as<bool>;

		/**
		 * \brief Updates the state of the decoder
		 */
		{decoder.set_state(std::declval<typename T::state>())} -> std::same_as<void>;

		/** \name stringdec_state
		 *  The different state decoder can be in
		 */
		///@{
		{T::state::string_escape} -> std::same_as<typename T::state>;
		{T::state::decode_codepoint} -> std::same_as<typename T::state>;
		{T::state::string_normal} -> std::same_as<typename T::state>;
		{T::state::before_second_surrogate_element_escseq} -> std::same_as<typename T::state>;
		{T::state::at_second_surrogate_element_escseq} -> std::same_as<typename T::state>;
		///@}
	};

	/**
	 * \brief Handles the `normal` state when decoding a quoted string
	 *
	 * When
	 *
	 * * `ch_in` equals StringDecoder::string_escape_char (as described in quoted_string_decoder), the
	 * decoder state is updated to `string_escape`, and no result is returned
	 *
	 * * `ch_in` equals StringDecoder::quoted_string_end_char (as described in quoted_string_decoder), the decoder
	 * state is kept, and the function returns quoted_string_decode_result::done
	 *
	 * * In any other case, decoder is asked to `accept_char` (as described in quoted_string_decoder). If
	 * decoder accepts `ch_in`, no result is returned, otherwise the return value is
	 * quoted_string_decode_result::illegal_char_in_string. In any case, the decoder state is not updated.
	 *
	 * \ingroup stringconv
	 */
	template<quoted_string_decoder StringDecoder>
	constexpr std::optional<quoted_string_decode_result> decode_quoted_string_state_normal(
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

			case StringDecoder::quoted_string_end_char:
				return quoted_string_decode_result::done;

			default:
				if(!decoder.accept_char(ch_in))
				{ return quoted_string_decode_result::illegal_char_in_string; }
				return std::nullopt;
		}
	}

	/**
	 * \brief  Handles the `string_escape` state when decoding a quoted string
	 *
	 * When
	 *
	 * * `ch_in` equals StringDecoder::codepoint_begin_char (as described in quoted_string_decoder), the
	 * decoder state is updated to `decode_codepoint`, and no result is returned
	 *
	 * * Otherwise, decoder is asked to `accept_escape_code` (as described in quoted_string_decoder). If
	 * decoder accepts `ch_in`, decoder state is updated to `string_normal` and there is no return
	 * value. If it was not accepted, quoted_string_decode_result::illegal_char_in_string is returned.
	 *
	 * \ingroup stringconv
	 */
	template<quoted_string_decoder StringDecoder>
	constexpr std::optional<quoted_string_decode_result> decode_quoted_string_state_string_escape(
		StringDecoder& decoder,
		char ch_in
	)
	{
		using decoder_state = StringDecoder::state;
		switch(ch_in)
		{
			case StringDecoder::codepoint_begin_char:
				decoder.set_state(decoder_state::decode_codepoint);
				return std::nullopt;

			default:
				if(!decoder.accept_escape_code(ch_in))
				{ return quoted_string_decode_result::illegal_char_in_string; }

				decoder.set_state(decoder_state::string_normal);
				return std::nullopt;
		}
	}

	/**
	 * \brief  Handles the `decode_codepoint` state when decoding a quoted string
	 *
	 * This function will call accept_codepoint_hexdigit (see quoted_string_decoder) on decoder, when the
	 * result is
	 *
	 * * accept_codepoint_hexdigit_result::done, the decoder state is updated to `string_normal`,
	 * and the function returns nothing
	 *
	 * * accept_codepoint_hexdigit_result::illegal_char_in_string, the function returns
	 * quoted_string_decode_result::illegal_char_in_string
	 *
	 * * accept_codepoint_hexdigit_result::invalid_input, the function returns
	 * quoted_string_decode_result::invalid_escape_sequence
	 *
	 * * accept_codepoint_hexdigit_result::second_surrogate_element_required, the decoder state is
	 * updated to `before_second_surrogate_element_escseq` and the function returns nothing
	 *
	 * \ingroup stringconv
	 */
	template<quoted_string_decoder StringDecoder>
	constexpr std::optional<quoted_string_decode_result> decode_quoted_string_state_decode_codepoint(
		StringDecoder& decoder,
		char ch_in
	)
	{
		using decoder_state = StringDecoder::state;
		switch(decoder.accept_codepoint_hexdigit(ch_in))
		{
			case accept_codepoint_hexdigit_result::done:
				decoder.set_state(decoder_state::string_normal);
				return std::nullopt;

			case accept_codepoint_hexdigit_result::illegal_char_in_string:
				return quoted_string_decode_result::illegal_char_in_string;

			case accept_codepoint_hexdigit_result::invalid_input:
				return quoted_string_decode_result::invalid_escape_sequence;

			case accept_codepoint_hexdigit_result::second_surrogate_element_required:
				decoder.set_state(decoder_state::before_second_surrogate_element_escseq);
				return std::nullopt;
		}
		raise_internal_error("Unexpected return value from accept_codepoint_hexdigit");
	}

	/**
	 * \brief Handles the `before_second_surrogate_element_escseq` state when decoding a quoted string
	 *
	 * When
	 *
	 * * `ch_in` equals `string_escape_char` (as described in quoted_string_decoder), the decoder state is
	 * updated to `at_second_surrogate_element_escseq` and  the function returns nothing
	 *
	 * * Otherwise quoted_string_decode_result::illformed_escaped_utf16_surrogate_pair is returned
	 *
	 * \ingroup stringconv
	 */
	template<quoted_string_decoder StringDecoder>
	constexpr std::optional<quoted_string_decode_result> decode_quoted_string_state_before_second_surrogate_element_escseq(
			StringDecoder& decoder,
			char ch_in
	)
	{
		using decoder_state = StringDecoder::state;
		switch(ch_in)
		{
			case StringDecoder::string_escape_char:
				decoder.set_state(decoder_state::at_second_surrogate_element_escseq);
				return std::nullopt;

			default:
				return quoted_string_decode_result::illformed_escaped_utf16_surrogate_pair;
		}
	}

	/**
	 * \brief Handles the `at_second_surrogate_element_escseq` state when decoding a quoted string
	 *
	 * When
	 *
	 * * `ch_in`  equals `codepoint_begin_char` (as described in quoted_string_decoder), the decoder state is
	 * updated to `decode_codepoint` and the function returns nothing
	 *
	 * * Otherwise, the function returns quoted_string_decode_result::illformed_escaped_utf16_surrogate_pair
	 *
	 * \ingroup stringconv
	 */
	template<class StringDecoder>
	constexpr std::optional<quoted_string_decode_result> decode_quoted_string_state_at_second_surrogate_element_escseq(
		StringDecoder& decoder,
		char ch_in
	)
	{
		using decoder_state = StringDecoder::state;
		switch(ch_in)
		{
			case StringDecoder::codepoint_begin_char:
					decoder.set_state(decoder_state::decode_codepoint);
					return std::nullopt;

			default:
					return quoted_string_decode_result::illformed_escaped_utf16_surrogate_pair;
		}
	}

	/**
	 * \brief Decodes a quoted string using the provided stateful decoder
	 *
	 * This function iterates over the input range obtained from `decoder.get_input_range()`,
	 * processing characters according to the current decoder state (such as normal text, escape
	 * sequences, Unicode codepoints, or surrogate pairs). As characters are consumed and states
	 * transition, the decoder's input range and internal state are updated accordingly.
	 *
	 * The state diagram for the decoding procedure is presented below
	 *
	 * @startuml
	 * hide empty description
	 *
	 * [*] --> string_normal : Start decoding
	 *
	 * state string_normal {
	 * }
	 *
	 * state string_escape {
	 * }
	 *
	 * state decode_codepoint {
	 * }
	 *
	 * state before_second_surrogate_element_escseq {
	 * }
	 *
	 * state at_second_surrogate_element_escseq {
	 * }
	 *
	 * string_normal --> string_escape : string_escape_char ('\\')
	 * string_normal --> [*] : quoted_string_end_char ('"') [done]
	 * string_normal --> string_normal : accept_char() [valid normal char]
	 *
	 * string_escape --> decode_codepoint : codepoint_begin_char ('u')
	 * string_escape --> string_normal : accept_escape_code() [valid escape]
	 *
	 * decode_codepoint --> string_normal : hexdigit::done
	 * decode_codepoint --> before_second_surrogate_element_escseq : hexdigit::second_surrogate_required
	 *
	 * before_second_surrogate_element_escseq --> at_second_surrogate_element_escseq : string_escape_char ('\\')
	 * at_second_surrogate_element_escseq --> decode_codepoint : codepoint_begin_char ('u')
	 * @enduml
	 *
	 * \tparam StringDecoder A type satisfying the \ref quoted_string_decoder concept.
	 * \param decoder The stateful decoder instance managing the input buffer and decoding state.
	 * \return A \ref quoted_string_decode_result indicating whether the operation completed successfully,
	 *         requires more data, or encountered a decoding error.
	 * \ingroup stringconv
	 * \warning This function only handles decoder states related to quoted strings. It is an error
	 *          to call this function if `decoder.get_state()` returns any other value than
	 *          documented in quoted_string_decoder.
	 */
	template<quoted_string_decoder StringDecoder>
	constexpr quoted_string_decode_result decode_quoted_string(StringDecoder& decoder)
	{
		auto src_range = decoder.get_input_range();
		using src_range_type = decltype(src_range);
		auto first = src_range.begin();
		auto const last = src_range.end();

		auto commit = [&](auto status){
			if(status == quoted_string_decode_result::done)
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
					if(auto const result = decode_quoted_string_state_normal(decoder, ch_in))
					{ return commit(*result); }
					break;

				case decoder_state::string_escape:
					if(auto const result = decode_quoted_string_state_string_escape(decoder, ch_in))
					{ return commit(*result); }
					break;

				case decoder_state::decode_codepoint:
					if(auto const result = decode_quoted_string_state_decode_codepoint(decoder, ch_in))
					{ return commit(*result); }
					break;

				case decoder_state::before_second_surrogate_element_escseq:
					if(
						auto const result = decode_quoted_string_state_before_second_surrogate_element_escseq(
							decoder,
							ch_in
						)
					)
					{ return commit(*result); }
					break;

				case decoder_state::at_second_surrogate_element_escseq:
					if(
						auto const result = decode_quoted_string_state_at_second_surrogate_element_escseq(
							decoder,
							ch_in
						)
					)
					{ return commit(*result); }
					break;

				default:
					raise_internal_error("Unexpected return value from get_state");
			}
#pragma GCC diagnostic pop
			++first;
		}
		return commit(quoted_string_decode_result::more_data_needed);
	}
}

#endif

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
	 * \brief The result of a string decode operation
	 * \ingroup stringconv
	 */
	enum class string_decode_result
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
	concept string_decoder = requires(T& decoder, char ch_in)
	{
		/**
		 * \brief The character used to start an escape sequence, typically `\`
		 */
		{T::string_escape_char} -> std::same_as<char const&>;

		/**
		 * \brief The character used to end a string, typically `"`
		 */
		{T::string_end_char} -> std::same_as<char const&>;

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
		 *        character, and store the result in an internal buffer
		 *
		 * \return true if `ch_in` was accepted, otherwise `false`
		 */
		{decoder.accept_escape_char(ch_in)} -> std::same_as<bool>;

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
	 * * `ch_in` equals StringDecoder::string_escape_char (as described in string_decoder), the
	 * decoder state is updated to `string_escape`, and no result is returned
	 *
	 * * `ch_in` equals StringDecoder::string_end_char (as described in string_decoder), the decoder
	 * state is kept, and the function returns string_decode_result::done
	 *
	 * * In any other case, decoder is asked to `accept_char` (as described in string_decoder). If
	 * decoder accepts `ch_in`, no result is returned, otherwise the return value is
	 * string_decode_result::illegal_char_in_string. In any case, the decoder state is not updated.
	 *
	 * \ingroup stringconv
	 */
	template<string_decoder StringDecoder>
	constexpr std::optional<string_decode_result> decode_quoted_string_state_normal(
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

			case StringDecoder::string_end_char:
				return string_decode_result::done;

			default:
				if(!decoder.accept_char(ch_in))
				{ return string_decode_result::illegal_char_in_string; }
				return std::nullopt;
		}
	}

	/**
	 * \brief  Handles the `string_escape` state when decoding a quoted string
	 *
	 * When
	 *
	 * * `ch_in` equals StringDecoder::codepoint_begin_char (as described in string_decoder), the
	 * decoder state is updated to `decode_codepoint`, and no result is returned
	 *
	 * * Otherwise, decoder is asked to `accept_escape_char` (as described in string_decoder). If
	 * decoder accepts `ch_in`, decoder state is updated to `string_normal` and there is no return
	 * value. If it was not accepted, string_decode_result::illegal_char_in_string is returned.
	 *
	 * \ingroup stringconv
	 */
	template<string_decoder StringDecoder>
	constexpr std::optional<string_decode_result> decode_quoted_string_state_string_escape(
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
				if(!decoder.accept_escape_char(ch_in))
				{ return string_decode_result::illegal_char_in_string; }

				decoder.set_state(decoder_state::string_normal);
				return std::nullopt;
		}
	}

	/**
	 * \brief  Handles the `decode_codepoint` state when decoding a quoted string
	 *
	 * This function will call accept_codepoint_hexdigit (see string_decoder) on decoder, when the
	 * result is
	 *
	 * * accept_codepoint_hexdigit_result::done, the decoder state is updated to `string_normal`,
	 * and the function returns nothing
	 *
	 * * accept_codepoint_hexdigit_result::illegal_char_in_string, the function returns
	 * string_decode_result::illegal_char_in_string
	 *
	 * * accept_codepoint_hexdigit_result::invalid_input, the function returns
	 * string_decode_result::invalid_escape_sequence
	 *
	 * * accept_codepoint_hexdigit_result::second_surrogate_element_required, the decoder state is
	 * updated to `before_second_surrogate_element_escseq` and the function returns nothing
	 *
	 * \ingroup stringconv
	 */
	template<class StringDecoder>
	constexpr std::optional<string_decode_result> decode_quoted_string_state_decode_codepoint(
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
				return string_decode_result::illegal_char_in_string;

			case accept_codepoint_hexdigit_result::invalid_input:
				return string_decode_result::invalid_escape_sequence;

			case accept_codepoint_hexdigit_result::second_surrogate_element_required:
				decoder.set_state(decoder_state::before_second_surrogate_element_escseq);
				return std::nullopt;
		}
		raise_internal_error("Unexpected return value from accept_codepoint_hexdigit");
	}

	/**
	 * \ingroup stringconv
	 */
	template<class StringDecoder>
	constexpr std::optional<string_decode_result> decode_quoted_string_state_before_second_surrogate_element_escseq(
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
				return string_decode_result::illformed_escaped_utf16_surrogate_pair;
		}
	}

	/**
	 * \ingroup stringconv
	 */
	template<class StringDecoder>
	constexpr std::optional<string_decode_result> decode_quoted_string_state_at_second_surrogate_element_escseq(
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
					return string_decode_result::illformed_escaped_utf16_surrogate_pair;
		}
	}

	/**
	 * \ingroup stringconv
	 */
	template<class StringDecoder>
	constexpr string_decode_result decode_quoted_string(StringDecoder& decoder)
	{
		auto src_range = decoder.get_input_range();
		using src_range_type = decltype(src_range);
		auto first = src_range.begin();
		auto const last = src_range.end();

		auto commit = [&](auto status){
			if(status == string_decode_result::done)
			{ ++first; }
			decoder.set_input_range(src_range_type{first, last});
			return status;
		};

		while(first != last)
		{
			auto const ch_in = *first;
			using decoder_state = StringDecoder::state;
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
			}
			++first;
		}
		return commit(string_decode_result::more_data_needed);
	}
}

#endif

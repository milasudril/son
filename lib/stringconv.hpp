#ifndef SON_STRINGCONV_HPP
#define SON_STRINGCONV_HPP

#include <optional>

/**
 * \defgroup stringconv String conversion utilities
 */

namespace son
{
	/**
	 * \ingroup stringconv
	 */
	enum class string_decode_result
	{
		done,
		illegal_char_in_string,
		invalid_escape_sequence,
		more_data_needed,
		illformed_escaped_utf16_surrogate_pair
	};

	/**
	 * \ingroup stringconv
	 */
	enum class accept_codepoint_hexdigit_result
	{
		done,
		illegal_char_in_string,
		invalid_input,
		second_surrogate_element_required
	};

	/**
	 * \ingroup stringconv
	 */
	template<class StringDecoder>
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
	 * \ingroup stringconv
	 */
	template<class StringDecoder>
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
		return std::nullopt;
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

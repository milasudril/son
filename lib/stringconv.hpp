#ifndef SON_STRINGCONV_HPP
#define SON_STRINGCONV_HPP

namespace son
{
	enum class string_decode_result
	{
		done,
		illegal_char_in_string,
		invalid_escape_sequence,
		more_data_needed,
		illformed_escaped_utf16_surrogate_pair
	};

	enum class accept_codepoint_hexdigit_result
	{
		done,
		illegal_char_in_string,
		invalid_input,
		second_surrogate_element_required
	};

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
					switch(ch_in)
					{
						case StringDecoder::string_escape_char:
							decoder.set_state(decoder_state::string_escape);
							break;

						case StringDecoder::string_end_char:
							return commit(string_decode_result::done);

						default:
							if(!decoder.accept_char(ch_in))
							{ return commit(string_decode_result::illegal_char_in_string); }
					}
					break;

				case decoder_state::string_escape:
					switch(ch_in)
					{
						case StringDecoder::codepoint_begin_char:
							decoder.set_state(decoder_state::decode_codepoint);
							break;

						default:
							if(!decoder.accept_escape_char(ch_in))
							{ return commit(string_decode_result::illegal_char_in_string); }

							decoder.set_state(decoder_state::string_normal);
					}
					break;

				case decoder_state::decode_codepoint:
					switch(decoder.accept_codepoint_hexdigit(ch_in))
					{
						case accept_codepoint_hexdigit_result::done:
							decoder.set_state(decoder_state::string_normal);
							break;

						case accept_codepoint_hexdigit_result::illegal_char_in_string:
							return commit(string_decode_result::illegal_char_in_string);

						case accept_codepoint_hexdigit_result::invalid_input:
							return commit(string_decode_result::invalid_escape_sequence);

						case accept_codepoint_hexdigit_result::second_surrogate_element_required:
							decoder.set_state(decoder_state::before_second_surrogate_element_escseq);
							break;
					}
					break;

				case decoder_state::before_second_surrogate_element_escseq:
					switch(ch_in)
					{
						case StringDecoder::string_escape_char:
							decoder.set_state(decoder_state::at_second_surrogate_element_escseq);
							break;

						default:
							return commit(string_decode_result::illformed_escaped_utf16_surrogate_pair);
					}
					break;

				case decoder_state::at_second_surrogate_element_escseq:
					switch(ch_in)
					{
						case StringDecoder::codepoint_begin_char:
							decoder.set_state(decoder_state::decode_codepoint);
							break;

						default:
							return commit(string_decode_result::illformed_escaped_utf16_surrogate_pair);
					}
					break;
			}
			++first;
		}
		return commit(string_decode_result::more_data_needed);
	}
}

#endif

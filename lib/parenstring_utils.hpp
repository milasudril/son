#ifndef SON_PARENSTRING_UTILS_HPP
#define SON_PARENSTRING_UTILS_HPP

#include "./exception.hpp"

#include <optional>

namespace son
{
	/**
	 * \brief The result of a decoding a parenstring
	 * \ingroup stringconv
	 */
	enum class parenstring_decode_result{
		done,
		illegal_char_in_string,
		more_data_needed
	};

	template<class StringDecoder>
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

	template<class StringDecoder>
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

	template<class StringDecoder>
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

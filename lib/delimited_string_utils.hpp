#ifndef SON_STRINGCONV_HPP
#define SON_STRINGCONV_HPP

#include <algorithm>
#include <string_view>

/**
 * \defgroup qstrings Functionality related to quoted strings
 */

namespace son
{
	/**
	 * \brief The result of a decoding a quoted string
	 * \ingroup qstrings
	 */
	enum class delimited_string_decode_result
	{
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
		illegal_char_in_string
	};

	/**
	 * \brief Requirements on a class that can manage the state during the decoding of a string
	 * \ingroup qstrings
	 */
	template<class T>
	concept delimited_string_decoder = requires(T& decoder, char ch_in)
	{
		/**
		 * \brief The character used to start an escape sequence, typically `\`
		 */
		{T::string_delimiters} -> std::same_as<std::string_view&>;

		/**
		 * \brief Should store `ch_in` in an internal buffer, if it is accepted
		 * \return true if `ch_in` was accepted, otherwise `false`
		 */
		{decoder.accept_char(ch_in)} -> std::same_as<bool>;
	};

	/**
	 * \brief Decodes a delimeted string using decoder, which manage the decoder state and input buffer
	 */
	template<delimited_string_decoder StringDecoder>
	constexpr delimited_string_decode_result decode_delimited_string(StringDecoder& decoder)
	{
		auto src_range = decoder.get_input_range();
		using src_range_type = decltype(src_range);
		auto first = src_range.begin();
		auto const last = src_range.end();

		auto commit = [&](auto status){
			if(status == delimited_string_decode_result::done)
			{ ++first; }
			decoder.set_input_range(src_range_type{first, last});
			return status;
		};

		while(first != last)
		{
			auto const ch_in = *first;
			if(std::ranges::any_of(StringDecoder::string_delimiters, ch_in))
			{ return commit(delimited_string_decode_result::done); }

			if(!decoder.accept_char(ch_in))
			{ return commit(delimited_string_decode_result::illegal_char_in_string); }

			++first;
		}
		return commit(delimited_string_decode_result::more_data_needed);
	}
}

#endif

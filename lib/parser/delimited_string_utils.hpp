#ifndef SON_STRINGCONV_HPP
#define SON_STRINGCONV_HPP

#include <concepts>

/**
 * \defgroup delimstring Delimited strings
 * \brief Functionality related to delimited strings
 */

namespace son
{
	/**
	 * \brief The result of a decoding a quoted string
	 * \ingroup delimstring
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
	 * \ingroup delimstring
	 */
	template<class T>
	concept delimited_string_decoder = requires(T& decoder, char ch_in)
	{
		/**
		 * \brief Should determine whether or not ch_in is a delimiter
		 */
		{decoder.is_string_delimiter(ch_in)} -> std::same_as<bool>;

		/**
		 * \brief Should store `ch_in` in an internal buffer, if it is accepted
		 * \return true if `ch_in` was accepted, otherwise `false`
		 */
		{decoder.accept_char(ch_in)} -> std::same_as<bool>;

		/**
		 * \brief Returns the current input range
		 */
		{decoder.get_input_range()};

		/**
		 * \brief Should update the input range in decoder
		 */
		{
			decoder.set_input_range(
				std::declval<decltype(decoder.get_input_range())>()
			)
		} -> std::same_as<void>;
	};

	/**
	 * \brief Decodes a delimeted string using decoder, which manage the decoder state and input buffer
	 * \ingroup delimstring
	 */
	template<delimited_string_decoder StringDecoder>
	constexpr delimited_string_decode_result decode_delimited_string(StringDecoder& decoder)
	{
		auto src_range = decoder.get_input_range();
		using src_range_type = decltype(src_range);
		auto first = src_range.begin();
		auto const last = src_range.end();

		auto commit = [&](auto status){
			decoder.set_input_range(src_range_type{first, last});
			return status;
		};

		while(first != last)
		{
			auto const ch_in = *first;
			if(decoder.is_string_delimiter(ch_in))
			{ return commit(delimited_string_decode_result::done); }

			if(!decoder.accept_char(ch_in))
			{ return commit(delimited_string_decode_result::illegal_char_in_string); }

			++first;
		}
		return commit(delimited_string_decode_result::more_data_needed);
	}
}

#endif

#ifndef SON_STRINGCONV_HPP
#define SON_STRINGCONV_HPP

#include <concepts>

/**
 * \defgroup charskip Character skip
 * \brief Functionality related character skipping
 */

namespace son
{
	/**
	 * \brief The result of a skip_chars operation
	 * \ingroup delimstring
	 */
	enum class skip_chars_result
	{
		/**
		 * \brief The operation was completed without errors
		 */
		done,

		/**
		 * \brief An invalid escape sequence appeared in the input buffer
		 */
		more_data_needed
	};

	/**
	 * \brief Requirements on a class that can manage the state during the decoding of a string
	 * \ingroup delimstring
	 */
	template<class T>
	concept char_skipper = requires(T& decoder, char ch_in)
	{
		/**
		 * \brief Returns `true` when ch_in should be skipped
		 */
		{decoder.skip_char(ch_in)} -> std::same_as<bool>;

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
	 * \ingroup charskip
	 */
	template<char_skipper StringDecoder>
	constexpr skip_chars_result skip_chars(StringDecoder& decoder)
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
			if(!decoder.skip_char(ch_in))
			{ return commit(skip_chars_result::done); }

			++first;
		}
		return commit(skip_chars_result::more_data_needed);
	}
}

#endif

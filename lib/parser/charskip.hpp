#ifndef SON_STRINGCONV_HPP
#define SON_STRINGCONV_HPP

#include "lib/core/operation_result.hpp"
#include "lib/core/common_traits.hpp"

#include <concepts>
#include <optional>

/**
 * \defgroup charskip Character skip
 * \brief Functionality related character skipping
 */

namespace son
{
	/**
	 * \brief Requirements on a class that can manage the state during the decoding of a string
	 * \ingroup delimstring
	 */
	template<class T>
	concept char_skipper = requires(T& decoder, char ch_in)
	{
		{decoder.update_stream_state(ch_in)} -> instantiation_of<std::optional>;

		/**
		 * \brief Returns a status code when the stream state was updated
		 */
		{*decoder.update_stream_state(ch_in)} -> explicitly_convertible_to<operation_result>;

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
	constexpr operation_result skip_chars(StringDecoder& decoder)
	{
		auto src_range = decoder.get_input_range();
		using src_range_type = decltype(src_range);
		auto first = src_range.begin();
		auto const last = src_range.end();

		while(first != last)
		{
			auto const ch_in = *first;
			if(auto const res = decoder.update_stream_state(ch_in))
			{
				++first;
				decoder.set_input_range(src_range_type{first, last});
				return operation_result{*res};
			}

			if(!decoder.skip_char(ch_in))
			{
				decoder.set_input_range(src_range_type{first, last});
				return operation_result{operation_result_value::completed};
			}

			++first;
		}

		decoder.set_input_range(src_range_type{first, last});
		return operation_result{operation_result_value::more_data_needed};
	}
}

#endif

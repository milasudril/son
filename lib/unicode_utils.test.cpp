//@	{"target":{"name":"unicode_utils.test"}}

#include "./unicode_utils.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(son_unicode_utils_utf16_decoder_surrogate_intervals)
{
	EXPECT_EQ(son::utf16_decoder::low_surrogate_min, static_cast<char16_t>(0xDC00));
	EXPECT_EQ(son::utf16_decoder::low_surrogate_max, static_cast<char16_t>(0xDFFF));
	EXPECT_EQ(son::utf16_decoder::high_surrogate_min, static_cast<char16_t>(0xD800));
	EXPECT_EQ(son::utf16_decoder::high_surrogate_max, static_cast<char16_t>(0xDBFF));

	for(size_t k = 0; k != 0x10000; ++k)
	{
		EXPECT_EQ(
			son::utf16_decoder::is_low_surrogate(static_cast<char16_t>(k))
			, k >= 0xDC00 && k <= 0xDFFF
		);

		EXPECT_EQ(
			son::utf16_decoder::is_high_surrogate(static_cast<char16_t>(k))
			, k >= 0xD800 && k <= 0xDBFF
		);
	}
}

TESTCASE(son_unicode_utils_utf16_decoder_combine_surrogate_pair)
{
	auto const result_a = son::utf16_decoder::combine_surrogate_pair(
		std::array{
			static_cast<char16_t>(0xD800),
			static_cast<char16_t>(0xDC00)
		}
	);
	EXPECT_EQ(result_a, 65536);

	auto const result_b = son::utf16_decoder::combine_surrogate_pair(
		std::array{
			static_cast<char16_t>(0xDBFF),
			static_cast<char16_t>(0xDFFF)
		}
	);
	EXPECT_EQ(result_b, 1114111);
}

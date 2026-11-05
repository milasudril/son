//@	{"target":{"name":"unicode_utils.test"}}

#include "./unicode_utils.hpp"

#include <testfwk/death_test.hpp>
#include <testfwk/testfwk.hpp>
#include <memory>

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
	EXPECT_EQ(result_b, static_cast<char32_t>(1114111));
}

TESTCASE(son_unicode_utils_utf16_decoder_try_to_make_codepoint_first_is_low_surrogate)
{
	son::utf16_decoder decoder;
	EXPECT_EQ(decoder.is_in_initial_state(), true);
	auto const result = decoder.try_to_make_codepoint(son::utf16_decoder::low_surrogate_min);
	EXPECT_EQ(result.error(), son::utf16_decoder::result::low_surrogate_as_first_code_unit);
	EXPECT_EQ(decoder.is_in_initial_state(), true);
}

TESTCASE(son_unicode_utils_utf16_decoder_try_to_make_codepoint_char_within_bmp)
{
	son::utf16_decoder decoder;
	EXPECT_EQ(decoder.is_in_initial_state(), true);
	auto const result = decoder.try_to_make_codepoint(static_cast<char16_t>(0x4353));
	EXPECT_EQ(result.value(), static_cast<char32_t>(0x4353));
	EXPECT_EQ(decoder.is_in_initial_state(), true);
}

TESTCASE(son_unicode_utils_utf16_decoder_try_to_make_codepoint_second_code_unit_not_low_surrogate)
{
	son::utf16_decoder decoder;
	EXPECT_EQ(decoder.is_in_initial_state(), true);
	auto const result_1 = decoder.try_to_make_codepoint(static_cast<char16_t>(0xD800));
	EXPECT_EQ(result_1.error(), son::utf16_decoder::result::more_data_needed);
	EXPECT_EQ(decoder.is_in_initial_state(), false);

	auto const result_2 = decoder.try_to_make_codepoint(static_cast<char16_t>(0xD24));
	EXPECT_EQ(result_2.error(), son::utf16_decoder::result::second_code_unit_is_not_a_low_surrogate);
	EXPECT_EQ(decoder.is_in_initial_state(), true);
}

TESTCASE(son_unicode_utils_utf16_decoder_try_to_make_codepoint_outside_bmp)
{
	son::utf16_decoder decoder;
	EXPECT_EQ(decoder.is_in_initial_state(), true);
	auto const result_1 = decoder.try_to_make_codepoint(static_cast<char16_t>(0xD800));
	EXPECT_EQ(result_1.error(), son::utf16_decoder::result::more_data_needed);
	EXPECT_EQ(decoder.is_in_initial_state(), false);

	auto const result_2 = decoder.try_to_make_codepoint(static_cast<char16_t>(0xDC00));
	EXPECT_EQ(result_2.value(), static_cast<char32_t>(65536));
	EXPECT_EQ(decoder.is_in_initial_state(), true);
}

TESTCASE(son_unicode_utils_utf16_decoder_reset)
{
	son::utf16_decoder decoder;
	EXPECT_EQ(decoder.is_in_initial_state(), true);
	auto const result_1 = decoder.try_to_make_codepoint(static_cast<char16_t>(0xD800));
	EXPECT_EQ(result_1.error(), son::utf16_decoder::result::more_data_needed);
	EXPECT_EQ(decoder.is_in_initial_state(), false);
	decoder.reset();
	EXPECT_EQ(decoder.is_in_initial_state(), true);
}

TESTCASE(son_unicode_utils_utf16_decoder_corrupt_state)
{
	std::array<char, 6> bytes{0x02, 0x00, 0x01, 0x02, 0x03, 0x04};
	auto const decoder = std::start_lifetime_as<son::utf16_decoder>(&bytes);
	TestFwk::expect_death(
		[&decoder]{
			std::ignore = decoder->try_to_make_codepoint(static_cast<char16_t>(0xD800));
		},
		"son internal error: lib/./unicode_utils.hpp:129: Too many elements in UTF-16 codepoint\n",
		SIGABRT
	);
}

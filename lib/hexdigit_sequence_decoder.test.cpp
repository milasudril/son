//@	{"target":{"name":"hexdigit_sequence_decoder.test"}}

#include "./hexdigit_sequence_decoder.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(son_numconv_hexdigit_sequence_decoder_to_nibble_input_value_below_digits)
{
	for(size_t k = 0; k < '0'; ++k)
	{
		auto const res = son::to_nibble(static_cast<unsigned char>(k));
		EXPECT_EQ(res.has_value(), false);
	}
}

TESTCASE(son_numconv_hexdigit_sequence_decoder_to_nibble_input_value_within_digits)
{
	for(auto k = '0'; k <= '9'; ++k)
	{
		auto const res = son::to_nibble(static_cast<unsigned char>(k));
		EXPECT_EQ(res.has_value(), true);
		EXPECT_EQ(res.value(), static_cast<std::byte>(k - '0'));
	}
}

TESTCASE(son_numconv_hexdigit_sequence_decoder_to_nibble_input_value_between_digits_and_A)
{
	for(auto k = static_cast<char>('9' + 1); k < 'A'; ++k)
	{
		auto const res = son::to_nibble(static_cast<unsigned char>(k));
		EXPECT_EQ(res.has_value(), false);
	}
}

TESTCASE(son_numconv_hexdigit_sequence_decoder_to_nibble_input_value_within_A_to_F)
{
	for(auto k = 'A'; k <= 'F'; ++k)
	{
		auto const res = son::to_nibble(static_cast<unsigned char>(k));
		EXPECT_EQ(res.has_value(), true);
		EXPECT_EQ(res.value(), static_cast<std::byte>(10 +  k - 'A'));
	}
}

TESTCASE(son_numconv_hexdigit_sequence_decoder_to_nibble_input_value_between_A_to_F_and_a_z)
{
	for(auto k = static_cast<char>('F' + 1); k < 'a'; ++k)
	{
		auto const res = son::to_nibble(static_cast<unsigned char>(k));
		EXPECT_EQ(res.has_value(), false);
	}
}

TESTCASE(son_numconv_hexdigit_sequence_decoder_to_nibble_input_value_within_a_to_f)
{
	for(auto k = 'a'; k <= 'f'; ++k)
	{
		auto const res = son::to_nibble(static_cast<unsigned char>(k));
		EXPECT_EQ(res.has_value(), true);
		EXPECT_EQ(res.value(), static_cast<std::byte>(10 + k - 'a'));
	}
}

TESTCASE(son_numconv_hexdigit_sequence_decoder_to_nibble_input_value_above_all)
{
	for(auto k = 'f' + 1; k <= 255; ++k)
	{
		auto const res = son::to_nibble(static_cast<unsigned char>(k));
		EXPECT_EQ(res.has_value(), false);
	}
}

TESTCASE(son_numconv_hexdigit_sequence_decoder_try_to_make_value_immediate_endmarker)
{
	son::hexdigit_sequence_decoder<uint32_t, '.'> decoder;
	EXPECT_EQ(decoder.endmarker, '.');
	EXPECT_EQ(decoder.is_in_initial_state(), true);
	auto const res = decoder.try_to_make_value('.');
	EXPECT_EQ(res.value(), 0);
	EXPECT_EQ(decoder.is_in_initial_state(), true);
}

TESTCASE(son_numconv_hexdigit_sequence_decoder_try_to_make_value_endmarker_at_odd_postion)
{
	son::hexdigit_sequence_decoder<uint32_t, '.'> decoder;
	EXPECT_EQ(decoder.endmarker, '.');
	EXPECT_EQ(decoder.is_in_initial_state(), true);

	auto const res1 = decoder.try_to_make_value('A');
	EXPECT_EQ(res1.error(),  son::hexdigit_sequence_decode_result::more_data_needed);
	EXPECT_EQ(decoder.is_in_initial_state(), false);

	auto const res2 = decoder.try_to_make_value('.');
	EXPECT_EQ(res2.value(), 10);
	EXPECT_EQ(decoder.is_in_initial_state(), true);
}

TESTCASE(son_numconv_hexdigit_sequence_decoder_decode_one_byte)
{
	son::hexdigit_sequence_decoder<uint32_t, '.'> decoder;
	EXPECT_EQ(decoder.endmarker, '.');
	EXPECT_EQ(decoder.is_in_initial_state(), true);

	static constexpr std::string_view input_vals{"12"};
	for(auto item : input_vals)
	{
		auto const res = decoder.try_to_make_value(item);
		EXPECT_EQ(res.error(),  son::hexdigit_sequence_decode_result::more_data_needed);
		EXPECT_EQ(decoder.is_in_initial_state(), false);
	}

	auto const res = decoder.try_to_make_value('.');
	EXPECT_EQ(decoder.is_in_initial_state(), true);
	EXPECT_EQ(res.value(),  0x12);
}

TESTCASE(son_numconv_hexdigit_sequence_decoder_decode_two_bytes)
{
	son::hexdigit_sequence_decoder<uint32_t, '.'> decoder;
	EXPECT_EQ(decoder.endmarker, '.');
	EXPECT_EQ(decoder.is_in_initial_state(), true);

	static constexpr std::string_view input_vals{"1234"};
	for(auto item : input_vals)
	{
		auto const res = decoder.try_to_make_value(item);
		EXPECT_EQ(res.error(),  son::hexdigit_sequence_decode_result::more_data_needed);
		EXPECT_EQ(decoder.is_in_initial_state(), false);
	}

	auto const res = decoder.try_to_make_value('.');
	EXPECT_EQ(decoder.is_in_initial_state(), true);
	EXPECT_EQ(res.value(),  0x1234);
}

TESTCASE(son_numconv_hexdigit_sequence_decoder_decode_three_bytes)
{
	son::hexdigit_sequence_decoder<uint32_t, '.'> decoder;
	EXPECT_EQ(decoder.endmarker, '.');
	EXPECT_EQ(decoder.is_in_initial_state(), true);

	static constexpr std::string_view input_vals{"123456"};
	for(auto item : input_vals)
	{
		auto const res = decoder.try_to_make_value(item);
		EXPECT_EQ(res.error(),  son::hexdigit_sequence_decode_result::more_data_needed);
		EXPECT_EQ(decoder.is_in_initial_state(), false);
	}

	auto const res = decoder.try_to_make_value('.');
	EXPECT_EQ(decoder.is_in_initial_state(), true);
	EXPECT_EQ(res.value(),  0x123456);
}

TESTCASE(son_numconv_hexdigit_sequence_decoder_decode_full_value)
{
	son::hexdigit_sequence_decoder<uint32_t, '.'> decoder;
	EXPECT_EQ(decoder.endmarker, '.');
	EXPECT_EQ(decoder.is_in_initial_state(), true);

	static constexpr std::string_view input_vals{"1234567"};
	for(auto item : input_vals)
	{
		auto const res = decoder.try_to_make_value(item);
		EXPECT_EQ(res.error(),  son::hexdigit_sequence_decode_result::more_data_needed);
		EXPECT_EQ(decoder.is_in_initial_state(), false);
	}

	auto const res = decoder.try_to_make_value('8');
	EXPECT_EQ(decoder.is_in_initial_state(), true);
	EXPECT_EQ(res.value(),  0x12345678);
}

TESTCASE(son_numconv_hexdigit_sequence_decoder_bad_input)
{
	son::hexdigit_sequence_decoder<uint32_t, '.'> decoder;
	EXPECT_EQ(decoder.endmarker, '.');
	EXPECT_EQ(decoder.is_in_initial_state(), true);

	auto const res = decoder.try_to_make_value('l');
	EXPECT_EQ(res.error(),  son::hexdigit_sequence_decode_result::invalid_input);
	EXPECT_EQ(decoder.is_in_initial_state(), true);
}

TESTCASE(son_numconv_hexdigit_sequence_decoder_reset)
{
	son::hexdigit_sequence_decoder<uint32_t, '.'> decoder;
	EXPECT_EQ(decoder.endmarker, '.');
	EXPECT_EQ(decoder.is_in_initial_state(), true);

	static constexpr std::string_view input_vals{"1234567"};
	for(auto item : input_vals)
	{
		auto const res = decoder.try_to_make_value(item);
		EXPECT_EQ(res.error(),  son::hexdigit_sequence_decode_result::more_data_needed);
		EXPECT_EQ(decoder.is_in_initial_state(), false);
	}

	decoder.reset();
	EXPECT_EQ(decoder.is_in_initial_state(), true);
}

TESTCASE(son_numconv_hexdigit_sequence_decoder_corrupt_state)
{
	std::array<uint32_t, 2*sizeof(uint32_t)> data{0x0, 0x20};
	auto const decoder = std::start_lifetime_as<son::hexdigit_sequence_decoder<uint32_t, '.'>>(&data);
	TestFwk::expect_death(
		[&decoder]{
			std::ignore = decoder->try_to_make_value('A');
		},
		"son internal error: lib/./hexdigit_sequence_decoder.hpp:107: Too many digits in hexdigit sequence\n",
		SIGABRT
	);
}

//@	{"target":{"name":"hexdigit_sequence_decoder.test"}}

#include "./hexdigit_sequence_decoder.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(son_hexdigit_sequence_decoder_to_nibble_input_value_below_digits)
{
	for(size_t k = 0; k < '0'; ++k)
	{
		auto const res = son::to_nibble(static_cast<char>(k));
		EXPECT_EQ(res.has_value(), false);
	}
}

TESTCASE(son_hexdigit_sequence_decoder_to_nibble_input_value_within_digits)
{
	for(auto k = '0'; k <= '9'; ++k)
	{
		auto const res = son::to_nibble(k);
		EXPECT_EQ(res.has_value(), true);
		EXPECT_EQ(res.value(), static_cast<std::byte>(k - '0'));
	}
}

TESTCASE(son_hexdigit_sequence_decoder_to_nibble_input_value_between_digits_and_A)
{
	for(auto k = static_cast<char>('9' + 1); k < 'A'; ++k)
	{
		auto const res = son::to_nibble(k);
		EXPECT_EQ(res.has_value(), false);
	}
}

TESTCASE(son_hexdigit_sequence_decoder_to_nibble_input_value_within_A_to_F)
{
	for(auto k = 'A'; k <= 'F'; ++k)
	{
		auto const res = son::to_nibble(k);
		EXPECT_EQ(res.has_value(), true);
		EXPECT_EQ(res.value(), static_cast<std::byte>(10 +  k - 'A'));
	}
}

TESTCASE(son_hexdigit_sequence_decoder_to_nibble_input_value_between_A_to_F_and_a_z)
{
	for(auto k = static_cast<char>('F' + 1); k < 'a'; ++k)
	{
		auto const res = son::to_nibble(k);
		EXPECT_EQ(res.has_value(), false);
	}
}

TESTCASE(son_hexdigit_sequence_decoder_to_nibble_input_value_within_a_to_f)
{
	for(auto k = 'a'; k <= 'f'; ++k)
	{
		auto const res = son::to_nibble(k);
		EXPECT_EQ(res.has_value(), true);
		EXPECT_EQ(res.value(), static_cast<std::byte>(10 + k - 'a'));
	}
}

TESTCASE(son_hexdigit_sequence_decoder_to_nibble_input_value_above_all)
{
	for(auto k = 'f' + 1; k <= 255; ++k)
	{
		auto const res = son::to_nibble(static_cast<char>(k));
		EXPECT_EQ(res.has_value(), false);
	}
}

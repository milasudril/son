//@	{"target":{"name":"charskip.test"}}

#include "./charskip.hpp"

#include <testfwk/testfwk.hpp>
#include <testfwk/mock_util.hpp>

namespace
{
	struct test_string_decoder
	{
		TestFwk::mock_entry<std::string_view()> get_input_range;
		TestFwk::mock_entry<void(std::string_view)> set_input_range;
		TestFwk::mock_entry<bool(char)> skip_char;
	};
}

TESTCASE(son_charskip_empty_returns_more_data_needed)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{};
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view str){
			EXPECT_EQ(str.empty(), true);
		}
	);
	auto const result = son::skip_chars(decoder);
	EXPECT_EQ(result, son::skip_chars_result::more_data_needed);
}

TESTCASE(son_charskip_only_skipped_chars_returns_more_data_needed)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"    "};
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view str){
			EXPECT_EQ(str.empty(), true);
		}
	);
	decoder.skip_char.expect_call_with_action(
		[](char ch_in){
			EXPECT_EQ(ch_in, ' ');
			return true;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(4)
		}
	);
	auto const result = son::skip_chars(decoder);
	EXPECT_EQ(result, son::skip_chars_result::more_data_needed);
}

TESTCASE(son_charskip_input_range_points_to_first_nonskipped_char)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"    Foobar"};
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view str){
			EXPECT_EQ(str, "Foobar");
		}
	);
	decoder.skip_char.expect_call_with_action(
		[](char ch_in){
			return ch_in == ' ';
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(5)
		}
	);
	auto const result = son::skip_chars(decoder);
	EXPECT_EQ(result, son::skip_chars_result::done);
}

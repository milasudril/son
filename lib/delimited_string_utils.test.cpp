//@ {"target":{"name":"delimited_string_utils.test"}}

#include "./delimited_string_utils.hpp"

#include <testfwk/testfwk.hpp>
#include <testfwk/mock_util.hpp>

namespace
{
	struct test_string_decoder
	{
		TestFwk::mock_entry<std::string_view()> get_input_range;
		TestFwk::mock_entry<void(std::string_view)> set_input_range;
		TestFwk::mock_entry<bool(char)> is_string_delimiter;
		TestFwk::mock_entry<bool(char)> accept_char;
	};
}

TESTCASE(son_delimited_string_utils_empty_returns_more_data_needed)
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
	auto const result = son::decode_delimited_string(decoder);
	EXPECT_EQ(result, son::delimited_string_decode_result::more_data_needed);
}

TESTCASE(son_delimited_string_utils_run_until_delimiter)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"Foobar.More data"};
		}
	);

	std::string buffer;
	decoder.accept_char.expect_call_with_action(
		[&buffer](char ch_in){
			buffer += ch_in;
			return true;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(6)
		}
	);
	decoder.is_string_delimiter.expect_call_with_action(
		[](char ch_in) {
			return ch_in == '.';
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(7)
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view str){
			EXPECT_EQ(str, ".More data");
		}
	);

	auto const result = son::decode_delimited_string(decoder);
	EXPECT_EQ(result, son::delimited_string_decode_result::done);
	EXPECT_EQ(buffer, "Foobar");
}

TESTCASE(son_delimited_string_utils_char_not_accepted)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"Foobar.More data"};
		}
	);
	decoder.accept_char.expect_call_with_action(
		[](char){
			return false;
		}
	);
	decoder.is_string_delimiter.expect_call_with_action(
		[](char ch_in) {
			return ch_in == '.';
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view str){
			EXPECT_EQ(str, "Foobar.More data");
		}
	);

	auto const result = son::decode_delimited_string(decoder);
	EXPECT_EQ(result, son::delimited_string_decode_result::illegal_char_in_string);
}

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

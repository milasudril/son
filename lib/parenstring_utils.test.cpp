//@	{"target":{"name":"parenstring_utils.test"}}

#include "./parenstring_utils.hpp"

#include <testfwk/mock_util.hpp>
#include <testfwk/testfwk.hpp>

namespace
{
	struct test_string_decoder
	{
		enum class state{
			string_normal,
			string_escape
		};

		static constexpr auto parenstring_begin_char = '(';
		static constexpr auto parenstring_end_char = ')';
		static constexpr auto string_escape_char = '\\';

		TestFwk::mock_entry<std::string_view()> get_input_range;
		TestFwk::mock_entry<void(std::string_view)> set_input_range;
		TestFwk::mock_entry<state()> get_state;
		TestFwk::mock_entry<void(state)> set_state;
		TestFwk::mock_entry<bool(char)> accept_char;
		TestFwk::mock_entry<void()> inc_paren_count;
		TestFwk::mock_entry<size_t()> dec_paren_count;
	};
}

TESTCASE(son_quoted_string_utils_empty_returns_more_data_needed)
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
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::more_data_needed);
}

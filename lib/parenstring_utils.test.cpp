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

TESTCASE(son_parenstring_utils_empty_returns_more_data_needed)
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

TESTCASE(son_quoted_string_utils_state_string_normal_handle_escape_char)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"\\"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_normal;
		}
	);
	decoder.set_state.expect_call_with_action(
		[](auto state) {
			EXPECT_EQ(state, test_string_decoder::state::string_escape);
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range.empty(), true);
		}
	);
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::more_data_needed);
}

TESTCASE(son_quoted_string_utils_state_string_normal_handle_begin_char)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"("};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_normal;
		}
	);
	decoder.inc_paren_count.expect_call_with_action(
		[]{}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range.empty(), true);
		}
	);
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::more_data_needed);
}

TESTCASE(son_quoted_string_utils_state_string_normal_handle_last_end_char)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{")more data"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_normal;
		}
	);
	decoder.dec_paren_count.expect_call_with_action(
		[] -> size_t{
			return 0;
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range, "more data");
		}
	);
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::done);
}

TESTCASE(son_quoted_string_utils_state_string_normal_handle_end_char)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{")"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_normal;
		}
	);
	decoder.dec_paren_count.expect_call_with_action(
		[] -> size_t{
			return 1;
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range.empty(), true);
		}
	);
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::more_data_needed);
}

//@	{"target":{"name":"parenstring_utils.test"}}

#include "./parenstring_utils.hpp"

#include <testfwk/mock_util.hpp>
#include <testfwk/testfwk.hpp>
#include <algorithm>
#include <cstring>

namespace
{
	struct test_string_decoder
	{
		enum class state{
			string_normal,
			string_escape,
			bad
		};

		static constexpr auto parenstring_begin_char = '(';
		static constexpr auto parenstring_end_char = ')';
		static constexpr auto string_escape_char = '\\';
		static constexpr auto parenstring_escape_char_escape_code = '-';

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

TESTCASE(son_parenstring_utils_state_string_normal_handle_escape_char)
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

TESTCASE(son_parenstring_utils_state_string_normal_handle_begin_char)
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
	decoder.accept_char.expect_call_with_action([](auto ch_in){
		EXPECT_EQ(ch_in, '(');
		return true;
	});
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range.empty(), true);
		}
	);
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::more_data_needed);
}

TESTCASE(son_parenstring_utils_state_string_normal_handle_begin_char_char_not_accepted)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"(more data"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_normal;
		}
	);
	decoder.accept_char.expect_call_with_action([](auto ch_in){
		EXPECT_EQ(ch_in, '(');
		return false;
	});
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range, "(more data");
		}
	);
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::illegal_char_in_string);
}

TESTCASE(son_parenstring_utils_state_string_normal_handle_last_end_char)
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

TESTCASE(son_parenstring_utils_state_string_normal_handle_end_char)
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
	decoder.accept_char.expect_call_with_action([](char ch_in){
		EXPECT_EQ(ch_in, ')');
		return true;
	});
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range.empty(), true);
		}
	);
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::more_data_needed);
}

TESTCASE(son_parenstring_utils_state_string_normal_handle_end_char_char_not_accepted)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{")some data"};
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
	decoder.accept_char.expect_call_with_action([](char ch_in){
		EXPECT_EQ(ch_in, ')');
		return false;
	});
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range, ")some data");
		}
	);
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::illegal_char_in_string);
}

TESTCASE(son_parenstring_utils_state_string_normal_handle_other_char)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"X"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_normal;
		}
	);
	decoder.accept_char.expect_call_with_action(
		[](char ch_in){
			EXPECT_EQ(ch_in, 'X');
			return true;
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

TESTCASE(son_parenstring_utils_state_string_normal_handle_other_char_not_accepted)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"Some data"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_normal;
		}
	);
	decoder.accept_char.expect_call_with_action(
		[](char ch_in){
			EXPECT_EQ(ch_in, 'S');
			return false;
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range, "Some data");
		}
	);
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::illegal_char_in_string);
}

TESTCASE(son_parenstring_utils_state_string_normal_handle_other_char_accepted)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"X"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_normal;
		}
	);
	decoder.accept_char.expect_call_with_action(
		[](char ch_in){
			EXPECT_EQ(ch_in, 'X');
			return true;
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

TESTCASE(son_parenstring_utils_state_string_escape_handle_parenstring_begin_char_not_accepted)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"(Some data"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_escape;
		}
	);
	decoder.accept_char.expect_call_with_action(
		[](char ch_in){
			EXPECT_EQ(ch_in, '(');
			return false;
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range, "(Some data");
		}
	);
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::illegal_char_in_string);
}

TESTCASE(son_parenstring_utils_state_string_escape_handle_parenstring_begin_char_accepted)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"("};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_escape;
		}
	);
	decoder.accept_char.expect_call_with_action(
		[](char ch_in){
			EXPECT_EQ(ch_in, '(');
			return true;
		}
	);
	decoder.set_state.expect_call_with_action(
		[](auto new_state){
			EXPECT_EQ(new_state, test_string_decoder::state::string_normal);
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

TESTCASE(son_parenstring_utils_state_string_escape_handle_parenstring_end_char_not_accepted)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{")Some data"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_escape;
		}
	);
	decoder.accept_char.expect_call_with_action(
		[](char ch_in){
			EXPECT_EQ(ch_in, ')');
			return false;
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range, ")Some data");
		}
	);
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::illegal_char_in_string);
}

TESTCASE(son_parenstring_utils_state_string_escape_handle_parenstring_end_char_accepted)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{")"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_escape;
		}
	);
	decoder.accept_char.expect_call_with_action(
		[](char ch_in){
			EXPECT_EQ(ch_in, ')');
			return true;
		}
	);
	decoder.set_state.expect_call_with_action(
		[](auto new_state){
			EXPECT_EQ(new_state, test_string_decoder::state::string_normal);
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

TESTCASE(son_parenstring_utils_state_string_escape_handle_parenstring_escape_char_not_accepted)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"-Some data"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_escape;
		}
	);
	decoder.accept_char.expect_call_with_action(
		[](char ch_in){
			EXPECT_EQ(ch_in, '\\');
			return false;
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range, "-Some data");
		}
	);
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::illegal_char_in_string);
}

TESTCASE(son_parenstring_utils_state_string_escape_handle_parenstring_escape_char_accepted)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"-"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_escape;
		}
	);
	decoder.accept_char.expect_call_with_action(
		[](char ch_in){
			EXPECT_EQ(ch_in, '\\');
			return true;
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range.empty(), true);
		}
	);
	decoder.set_state.expect_call_with_action(
		[](auto new_state){
			EXPECT_EQ(new_state, test_string_decoder::state::string_normal);
		}
	);
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::more_data_needed);
}

TESTCASE(son_parenstring_utils_state_string_escape_handle_other_char_escape_char_not_accepted)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"\\some data"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_escape;
		}
	);
	decoder.accept_char.expect_call_with_action(
		[](char ch_in){
			EXPECT_EQ(ch_in, '\\');
			return false;
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range, "\\some data");
		}
	);
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::illegal_char_in_string);
}

TESTCASE(son_parenstring_utils_state_string_escape_handle_other_char_escape_char_accepted_following_not)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"some data"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_escape;
		}
	);
	decoder.accept_char.expect_call_with_action(
		[callcount = 0](char ch_in) mutable{
			++callcount;
			if(callcount == 1)
			{
				EXPECT_EQ(ch_in, '\\');
				return true;
			}

			EXPECT_EQ(ch_in, 's');
			return false;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(2)
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range, "some data");
		}
	);
	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::illegal_char_in_string);
}

TESTCASE(son_parenstring_utils_state_string_escape_handle_other_char_everything_accepted)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"s"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::string_escape;
		}
	);
	decoder.accept_char.expect_call_with_action(
		[callcount = 0](char ch_in) mutable{
			++callcount;
			if(callcount == 1)
			{
				EXPECT_EQ(ch_in, '\\');
				return true;
			}

			EXPECT_EQ(ch_in, 's');
			return true;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(2)
		}
	);
	decoder.set_state.expect_call_with_action(
		[](auto new_state){
			EXPECT_EQ(new_state, test_string_decoder::state::string_normal);
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

TESTCASE(son_parenstring_utils_state_bad)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"s"};
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(1),
			.count_with_fd = true
		}
	);
	decoder.get_state.expect_call_with_action(
		[]{
			return test_string_decoder::state::bad;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(1),
			.count_with_fd = true
		}
	);

	TestFwk::expect_death(
		[&]{
			std::ignore = son::decode_parenstring(decoder);
		},
		"son internal error: lib/parser/./parenstring_utils.hpp:364: Unexpected return value from get_state\n",
		SIGABRT
	);
}

TESTCASE(son_parenstring_utils_decode_string_with_all_parens_escaped)
{
	test_string_decoder decoder;
	static constexpr std::string_view to_decode{"I = \\(0, x\\(a + b\\)])remaining stuff"};
	static constexpr std::string_view decoded{"I = (0, x(a + b)]"};
	decoder.get_input_range.expect_call_with_action(
		[]{
			return to_decode;
		}
	);
	auto current_state = test_string_decoder::state::string_normal;
	decoder.get_state.expect_call_with_action(
		[&current_state]{
			return current_state;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(
				std::size(to_decode) - strlen("remaining stuff")
			)
		}
	);
	std::string buffer;
	decoder.accept_char.expect_call_with_action(
		[&buffer](char ch_in) {
			buffer += ch_in;
			return true;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(std::size(decoded))
		}
	);
	decoder.dec_paren_count.expect_call_with_action([]-> size_t{
		return 0;
	});
	decoder.set_state.expect_call_with_action(
		[&current_state](auto new_state){
			current_state = new_state;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(
				2*std::ranges::count(to_decode, '\\')
			)
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range) {
			EXPECT_EQ(new_range, "remaining stuff");
		}
	);

	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::done);
	EXPECT_EQ(buffer, decoded);
}

TESTCASE(son_parenstring_utils_decode_string_with_extra_open_paren)
{
	test_string_decoder decoder;
	static constexpr std::string_view to_decode{"I = \\(0, x(a + b)])remaining stuff"};
	static constexpr std::string_view decoded{"I = (0, x(a + b)]"};
	decoder.get_input_range.expect_call_with_action(
		[]{
			return to_decode;
		}
	);
	auto current_state = test_string_decoder::state::string_normal;
	decoder.get_state.expect_call_with_action(
		[&current_state]{
			return current_state;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(
				std::size(to_decode) - strlen("remaining stuff")
			)
		}
	);
	std::string buffer;
	decoder.accept_char.expect_call_with_action(
		[&buffer](char ch_in) {
			buffer += ch_in;
			return true;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(std::size(decoded))
		}
	);
	size_t num_parens = 1;
	decoder.inc_paren_count.expect_call_with_action(
		[&num_parens]{
			++num_parens;
		}
	);
	decoder.dec_paren_count.expect_call_with_action(
		[&num_parens]-> size_t{
			--num_parens;
			return num_parens;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(2)
		}
	);
	decoder.set_state.expect_call_with_action(
		[&current_state](auto new_state){
			current_state = new_state;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(
				2*std::ranges::count(to_decode, '\\')
			)
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range) {
			EXPECT_EQ(new_range, "remaining stuff");
		}
	);

	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::done);
	EXPECT_EQ(buffer, decoded);
}

TESTCASE(son_parenstring_utils_decode_string_with_extra_close_paren)
{
	test_string_decoder decoder;
	static constexpr std::string_view to_decode{"I = [0, x(a + b)\\))remaining stuff"};
	static constexpr std::string_view decoded{"I = [0, x(a + b))"};
	decoder.get_input_range.expect_call_with_action(
		[]{
			return to_decode;
		}
	);
	auto current_state = test_string_decoder::state::string_normal;
	decoder.get_state.expect_call_with_action(
		[&current_state]{
			return current_state;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(
				std::size(to_decode) - strlen("remaining stuff")
			)
		}
	);
	std::string buffer;
	decoder.accept_char.expect_call_with_action(
		[&buffer](char ch_in) {
			buffer += ch_in;
			return true;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(std::size(decoded))
		}
	);
	size_t num_parens = 1;
	decoder.inc_paren_count.expect_call_with_action(
		[&num_parens]{
			++num_parens;
		}
	);
	decoder.dec_paren_count.expect_call_with_action(
		[&num_parens]-> size_t{
			--num_parens;
			return num_parens;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(2)
		}
	);
	decoder.set_state.expect_call_with_action(
		[&current_state](auto new_state){
			current_state = new_state;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(
				2*std::ranges::count(to_decode, '\\')
			)
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range) {
			EXPECT_EQ(new_range, "remaining stuff");
		}
	);

	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::done);
	EXPECT_EQ(buffer, decoded);
}

TESTCASE(son_parenstring_utils_decode_string_with_escape_char_before_paren)
{
	test_string_decoder decoder;
	static constexpr std::string_view to_decode{"\\-())remaining stuff"};
	static constexpr std::string_view decoded{"\\()"};
	decoder.get_input_range.expect_call_with_action(
		[]{
			return to_decode;
		}
	);
	auto current_state = test_string_decoder::state::string_normal;
	decoder.get_state.expect_call_with_action(
		[&current_state]{
			return current_state;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(
				std::size(to_decode) - strlen("remaining stuff")
			)
		}
	);
	std::string buffer;
	decoder.accept_char.expect_call_with_action(
		[&buffer](char ch_in) {
			buffer += ch_in;
			return true;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(std::size(decoded))
		}
	);
	size_t num_parens = 1;
	decoder.inc_paren_count.expect_call_with_action(
		[&num_parens]{
			++num_parens;
		}
	);
	decoder.dec_paren_count.expect_call_with_action(
		[&num_parens]-> size_t{
			--num_parens;
			return num_parens;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(2)
		}
	);
	decoder.set_state.expect_call_with_action(
		[&current_state](auto new_state){
			current_state = new_state;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(
				2*std::ranges::count(to_decode, '\\')
			)
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range) {
			EXPECT_EQ(new_range, "remaining stuff");
		}
	);

	auto const result = son::decode_parenstring(decoder);
	EXPECT_EQ(result, son::parenstring_decode_result::done);
	EXPECT_EQ(buffer, decoded);
}

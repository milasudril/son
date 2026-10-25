//@	{"target":{"name":"stringconv.test"}}

#include "./stringconv.hpp"

#include <testfwk/testfwk.hpp>
#include <testfwk/mock_util.hpp>

namespace
{
	struct test_string_decoder
	{
		TestFwk::mock_entry<std::string_view()> get_input_range;
		TestFwk::mock_entry<void(std::string_view)> set_input_range;

		enum class state{
			string_normal,
			string_escape,
			decode_codepoint,
			before_second_surrogate_element_escseq,
			at_second_surrogate_element_escseq
		};

		TestFwk::mock_entry<state()> get_state;
		TestFwk::mock_entry<void(state)> set_state;
		TestFwk::mock_entry<bool(char)> accept_char;
		TestFwk::mock_entry<bool(char)> accept_escape_char;
		TestFwk::mock_entry<son::accept_codepoint_hexdigit_result(char)> accept_codepoint_hexdigit;

		static constexpr auto string_escape_char = '\\';
		static constexpr auto string_end_char = '\"';
		static constexpr auto codepoint_begin_char = 'u';
	};
}

TESTCASE(son_stringconv_empty_returns_more_data_needed)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action([](){return std::string_view{}; });
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view str){
			EXPECT_EQ(str.empty(), true);
		}
	);
	auto const result = son::decode_quoted_string(decoder);
	EXPECT_EQ(result, son::string_decode_result::more_data_needed);
}

TESTCASE(son_stringconv_state_string_normal_handle_escape_char)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[](){
			return std::string_view{"\\"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[](){
			return test_string_decoder::state::string_normal;
		}
	);
	decoder.set_state.expect_call_with_action(
		[](test_string_decoder::state new_state){
			EXPECT_EQ(new_state, test_string_decoder::state::string_escape);
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view str){
			EXPECT_EQ(str.empty(), true);
		}
	);
	auto const result = son::decode_quoted_string(decoder);
	EXPECT_EQ(result, son::string_decode_result::more_data_needed);
}

TESTCASE(son_stringconv_state_string_normal_handle_end_char)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[](){
			return std::string_view{"\"stuff after string"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[](){
			return test_string_decoder::state::string_normal;
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range, "stuff after string");
		}
	);
	auto const result = son::decode_quoted_string(decoder);
	EXPECT_EQ(result, son::string_decode_result::done);
}

TESTCASE(son_stringconv_state_string_normal_other_char_invalid)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[](){
			return std::string_view{"Some data"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[](){
			return test_string_decoder::state::string_normal;
		}
	);
	decoder.accept_char.expect_call_with_action([](char ch_in){
		EXPECT_EQ(ch_in, 'S');
		return false;
	});
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range, "Some data");
		}
	);
	auto const result = son::decode_quoted_string(decoder);
	EXPECT_EQ(result, son::string_decode_result::illegal_char_in_string);
}

TESTCASE(son_stringconv_state_string_normal_other_char_valid)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[](){
			return std::string_view{"Some data"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[](){
			return test_string_decoder::state::string_normal;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(9)
		}
	);
	std::string output_string;
	decoder.accept_char.expect_call_with_action(
		[&output_string](char ch_in){
			output_string += ch_in;
			return true;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(9)
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range.empty(), true);
		}
	);
	auto const result = son::decode_quoted_string(decoder);
	EXPECT_EQ(output_string, "Some data");
	EXPECT_EQ(result, son::string_decode_result::more_data_needed);
}

TESTCASE(son_stringconv_state_string_escape_codepoint_begin_char)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[](){
			return std::string_view{"u"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[](){
			return test_string_decoder::state::string_escape;
		}
	);
	decoder.set_state.expect_call_with_action(
		[](test_string_decoder::state new_state){
			EXPECT_EQ(new_state, test_string_decoder::state::decode_codepoint);
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range.empty(), true);
		}
	);
	auto const result = son::decode_quoted_string(decoder);
	EXPECT_EQ(result, son::string_decode_result::more_data_needed);
}

TESTCASE(son_stringconv_state_string_escape_other_char_invalid)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[](){
			return std::string_view{"Some data"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[](){
			return test_string_decoder::state::string_escape;
		}
	);
	decoder.accept_escape_char.expect_call_with_action([](char ch_in){
		EXPECT_EQ(ch_in, 'S');
		return false;
	});
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range, "Some data");
		}
	);
	auto const result = son::decode_quoted_string(decoder);
	EXPECT_EQ(result, son::string_decode_result::illegal_char_in_string);
}

TESTCASE(son_stringconv_state_string_escape_other_char_valid)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[](){
			return std::string_view{"n"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[](){
			return test_string_decoder::state::string_escape;
		}
	);
	decoder.accept_escape_char.expect_call_with_action(
		[](char ch_in){
			EXPECT_EQ(ch_in, 'n');
			return true;
		}
	);
	decoder.set_state.expect_call_with_action(
		[](test_string_decoder::state new_state){
			EXPECT_EQ(new_state, test_string_decoder::state::string_normal);
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range.empty(), true);
		}
	);
	auto const result = son::decode_quoted_string(decoder);
	EXPECT_EQ(result, son::string_decode_result::more_data_needed);
}

TESTCASE(son_stringconv_state_decode_codepoint_done)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[](){
			return std::string_view{"F"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[](){
			return test_string_decoder::state::decode_codepoint;
		}
	);
	decoder.accept_codepoint_hexdigit.expect_call_with_action(
		[](char ch_in){
			EXPECT_EQ(ch_in, 'F');
			return son::accept_codepoint_hexdigit_result::done;
		}
	);
	decoder.set_state.expect_call_with_action(
		[](test_string_decoder::state new_state){
			EXPECT_EQ(new_state, test_string_decoder::state::string_normal);
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range.empty(), true);
		}
	);
	auto const result = son::decode_quoted_string(decoder);
	EXPECT_EQ(result, son::string_decode_result::more_data_needed);
}

TESTCASE(son_stringconv_state_decode_codepoint_illegal_char_in_string)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[](){
			return std::string_view{"Some data"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[](){
			return test_string_decoder::state::decode_codepoint;
		}
	);
	decoder.accept_codepoint_hexdigit.expect_call_with_action(
		[](char ch_in){
			EXPECT_EQ(ch_in, 'S');
			return son::accept_codepoint_hexdigit_result::illegal_char_in_string;
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range, "Some data");
		}
	);
	auto const result = son::decode_quoted_string(decoder);
	EXPECT_EQ(result, son::string_decode_result::illegal_char_in_string);
}

TESTCASE(son_stringconv_state_decode_codepoint_invalid_input)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[](){
			return std::string_view{"Some data"};
		}
	);
	decoder.get_state.expect_call_with_action(
		[](){
			return test_string_decoder::state::decode_codepoint;
		}
	);
	decoder.accept_codepoint_hexdigit.expect_call_with_action(
		[](char ch_in){
			EXPECT_EQ(ch_in, 'S');
			return son::accept_codepoint_hexdigit_result::invalid_input;
		}
	);
	decoder.set_input_range.expect_call_with_action(
		[](std::string_view new_range){
			EXPECT_EQ(new_range, "Some data");
		}
	);
	auto const result = son::decode_quoted_string(decoder);
	EXPECT_EQ(result, son::string_decode_result::invalid_escape_sequence);
}

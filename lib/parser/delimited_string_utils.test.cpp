//@ {"target":{"name":"delimited_string_utils.test"}}

#include "./delimited_string_utils.hpp"
#include "lib/core/operation_result.hpp"

#include <testfwk/testfwk.hpp>
#include <testfwk/mock_util.hpp>

namespace
{
	enum class update_stream_state_result:son::half_uintptr_t{
		completed,
		aborted
	};

	constexpr son::operation_result_value convert_to(
		std::type_identity<son::operation_result_value> /*unused*/,
		update_stream_state_result value
	)
	{
		switch (value)
		{
			case update_stream_state_result::aborted:
				return son::operation_result_value::aborted;
			case update_stream_state_result::completed:
				return son::operation_result_value::completed;
		}
		throw std::runtime_error{"Unknown value"};
	}

	constexpr char const* to_string(update_stream_state_result value)
	{
		switch (value)
		{
			case update_stream_state_result::aborted:
				return "Aborted";
			case update_stream_state_result::completed:
				return "Completed";
		}
		throw std::runtime_error{"Unknown value"};
	}

	struct test_string_decoder
	{
		TestFwk::mock_entry<std::string_view()> get_input_range;
		TestFwk::mock_entry<void(std::string_view)> set_input_range;
		TestFwk::mock_entry<bool(char)> is_string_delimiter;
		TestFwk::mock_entry<bool(char)> accept_char;
		TestFwk::mock_entry<std::optional<update_stream_state_result>(char)> update_stream_state;
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
	EXPECT_EQ(result.status(), son::operation_result_value::more_data_needed);
}

TESTCASE(son_delimited_string_utils_run_until_delimiter)
{
	test_string_decoder decoder;
	static constexpr std::string_view input_range{"Foobar.More data"};
	decoder.get_input_range.expect_call_with_action(
		[]{
			return input_range;
		}
	);

	decoder.update_stream_state.expect_call_with_action(
		[ptr = input_range.data()](char ch_in) mutable {
			EXPECT_EQ(ch_in, *ptr);
			++ptr;
			return std::nullopt;
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(7)
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
	EXPECT_EQ(result.status(), son::operation_result_value::completed);
	EXPECT_EQ(buffer, "Foobar");
}

TESTCASE(son_delimited_string_utils_char_not_accepted)
{
	test_string_decoder decoder;
	static constexpr std::string_view input_range{"Foobar.More data"};
	decoder.get_input_range.expect_call_with_action(
		[]{
			return input_range;
		}
	);

	decoder.update_stream_state.expect_call_with_action(
		[ptr = input_range.data()](char ch_in) mutable {
			EXPECT_EQ(ch_in, *ptr);
			++ptr;
			return std::nullopt;
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
	EXPECT_EQ(result.error_description(), std::string_view{"Illegal char in string"});
}

TESTCASE(son_delimited_string_utils_stream_state_with_value)
{
	test_string_decoder decoder;
	static constexpr std::string_view input_range{"Foobar.More data"};
	decoder.get_input_range.expect_call_with_action(
		[]{
			return input_range;
		}
	);

	decoder.update_stream_state.expect_call_with_action(
		[ptr = input_range.data()](char ch_in) mutable {
			EXPECT_EQ(ch_in, *ptr);
			++ptr;
			return update_stream_state_result::aborted;
		}
	);

	decoder.set_input_range.expect_call_with_action(
		[](std::string_view str){
			EXPECT_EQ(str, "oobar.More data");
		}
	);

	auto const result = son::decode_delimited_string(decoder);
	EXPECT_EQ(result.error_description(), std::string_view{"Aborted"});
}

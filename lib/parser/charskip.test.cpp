//@	{"target":{"name":"charskip.test"}}

#include "./charskip.hpp"
#include "lib/core/int_types.hpp"
#include "lib/core/operation_result.hpp"
#include "testfwk/validation.hpp"

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
		TestFwk::mock_entry<bool(char)> skip_char;
		TestFwk::mock_entry<std::optional<update_stream_state_result>(char)> update_stream_state;
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
	EXPECT_EQ(result.status(), son::operation_result_value::more_data_needed);
}

TESTCASE(son_charskip_only_skipped_chars_returns_more_data_needed)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"    "};
		}
	);
	decoder.update_stream_state.expect_call_with_action(
		[](char ch_in) {
			EXPECT_EQ(ch_in, ' ');
			return std::optional<update_stream_state_result>{};
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(4)
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
	EXPECT_EQ(result.status(), son::operation_result_value::more_data_needed);
}

TESTCASE(son_charskip_input_range_points_to_first_nonskipped_char)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action(
		[]{
			return std::string_view{"    Foobar"};
		}
	);
	decoder.update_stream_state.expect_call_with_action(
		[callcount = 0](char ch_in) mutable {
			if(callcount < 4)
			{ EXPECT_EQ(ch_in, ' '); }
			else
			{ EXPECT_EQ(ch_in, 'F'); }
			++callcount;
			return std::optional<update_stream_state_result>{};
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(5)
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
	EXPECT_EQ(result.status(), son::operation_result_value::completed);
}

//@	{"target":{"name":"lexer.test"}}

#include "./lexer.hpp"
#include "lib/core/int_types.hpp"
#include "lib/core/operation_result.hpp"

#include <testfwk/testfwk.hpp>
#include <testfwk/mock_util.hpp>
#include <type_traits>

namespace
{
	struct test_lexer
	{
		TestFwk::mock_entry<son::operation_result()> process_current_token;
		TestFwk::mock_entry<std::optional<son::operation_result>()> begin_next_token;
	};

	enum class dummy_result:son::half_uintptr_t{
		done,
		more_data_needed
	};

	son::operation_result_value convert_to(
		std::type_identity<son::operation_result_value> /*unused*/,
		dummy_result res
	)
	{
		switch(res)
		{
			case dummy_result::done:
				return son::operation_result_value::completed;
			case dummy_result::more_data_needed:
				return son::operation_result_value::more_data_needed;
			default:
				abort();
		}
	}

	char const* to_string(dummy_result val)
	{
		switch(val)
		{
			case dummy_result::done:
				return "Completed";
			case dummy_result::more_data_needed:
				return "More data needed";
			default:
				abort();
		}
	}
}

TESTCASE(son_run_lexer_process_until_current_token_not_completed)
{
	test_lexer my_lexer;
	my_lexer.process_current_token.expect_call_with_action(
		[callcount = 0] mutable {
			if(callcount == 0)
			{
				++callcount;
				return son::operation_result{dummy_result::done};
			}
			return son::operation_result{dummy_result::more_data_needed};
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(2)
		}
	);
	my_lexer.begin_next_token.expect_call_with_action(
		[]{
			return std::nullopt;
		}
	);
	auto const res = son::run_lexer(my_lexer);
	EXPECT_NE(res.status(), son::operation_result_value::completed);
}

TESTCASE(son_run_lexer_process_until_current_token_completed)
{
	test_lexer my_lexer;
	my_lexer.process_current_token.expect_call_with_action(
		[] {
			return son::operation_result{dummy_result::done};
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(2)
		}
	);
	my_lexer.begin_next_token.expect_call_with_action(
		[callcount = 0] mutable {
			if(callcount == 0)
			{
				++callcount;
				return std::optional<son::operation_result>{};
			}
			return std::optional{son::operation_result{dummy_result::done}};
		},
		TestFwk::expectation_options{
			.cardinality = TestFwk::cardinality_constraint::exactly(2)
		}
	);
	auto const res = son::run_lexer(my_lexer);
	EXPECT_EQ(res.status(), son::operation_result_value::completed);
}


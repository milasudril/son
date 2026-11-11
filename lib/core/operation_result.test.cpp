//@	{"target":{"name":"operation_result.test"}}

#include "./operation_result.hpp"
#include "lib/core/int_types.hpp"

#include <csignal>
#include <testfwk/death_test.hpp>
#include <testfwk/testfwk.hpp>
#include <testfwk/mock_util.hpp>

namespace
{
	enum class test_result_type:son::half_uintptr_t{
		testval,
		testval2
	};

	TestFwk::mock_entry<
		son::operation_result_value(
			std::type_identity<son::operation_result_value>,
			test_result_type
		)
	> convert_to_mock;
	son::operation_result_value convert_to(
		std::type_identity<son::operation_result_value> tag,
		test_result_type value
	)
	{ return convert_to_mock(tag, value); }

	TestFwk::mock_entry<char const*(test_result_type)> to_string_mock;
	// NOLINTNEXTLINE: Accept crash if exception is not caught
	char const* to_string(test_result_type value) noexcept
	{ return to_string_mock(value); }
}

TESTCASE(son_operation_result_value_set)
{
	convert_to_mock.expect_call_with_action(
		[](
			std::type_identity<son::operation_result_value> /*unused*/,
			test_result_type value
		){
			EXPECT_EQ(value, test_result_type::testval2);
			return son::operation_result_value::suspended;
		}
	);
	son::operation_result res{test_result_type::testval2};
	EXPECT_EQ(res.status(), son::operation_result_value::suspended);

	to_string_mock.expect_call_with_action(
		[](test_result_type result){
			EXPECT_EQ(result, test_result_type::testval2);
			return "Foobar";
		}
	);
	auto const str = res.error_description();
	EXPECT_EQ(str, std::string_view{"Foobar"});
}

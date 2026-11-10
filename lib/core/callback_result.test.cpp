//@	{"target":{"name":"callback_result.test"}}

#include "./callback_result.hpp"
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
		son::callback_result::status_type(
			std::type_identity<son::callback_result::status_type>,
			test_result_type
		)
	> convert_to_mock;
	son::callback_result::status_type convert_to(
		std::type_identity<son::callback_result::status_type> tag,
		test_result_type value
	)
	{ return convert_to_mock(tag, value); }

	TestFwk::mock_entry<char const*(test_result_type)> to_string_mock;
	char const* to_string(test_result_type value)
	{ return to_string_mock(value); }
}

TESTCASE(son_callback_result_default_state)
{
	son::callback_result res;
	EXPECT_EQ(res.status(), son::callback_result::status_type::done);
	TestFwk::expect_death(
		[&](){
			std::ignore = res.error_description();
		},
		"son internal error: lib/core/./callback_result.hpp:38: callback_result has no error message\n",
		SIGABRT
	);
}

TESTCASE(son_callback_result_value_set)
{
	convert_to_mock.expect_call_with_action(
		[](
			std::type_identity<son::callback_result::status_type> /*unused*/,
			test_result_type value
		){
			EXPECT_EQ(value, test_result_type::testval2);
			return son::callback_result::status_type::more_data_needed;
		}
	);
	son::callback_result res{test_result_type::testval2};
	EXPECT_EQ(res.status(), son::callback_result::status_type::more_data_needed);

	to_string_mock.expect_call_with_action(
		[](test_result_type result){
			EXPECT_EQ(result, test_result_type::testval2);
			return "Foobar";
		}
	);
	auto const str = res.error_description();
	EXPECT_EQ(str, std::string_view{"Foobar"});
}

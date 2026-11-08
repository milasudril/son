//@	{"target":{"name":"callback_result.test"}}

#include "./callback_result.hpp"

#include <testfwk/testfwk.hpp>
#include <testfwk/mock_util.hpp>

namespace
{
	enum class test_result_type{
		testval
	};

	TestFwk::mock_entry_overload_set<
		son::callback_result::value_type(
			std::type_identity<son::callback_result::value_type>,
			test_result_type
		),
		test_result_type(
			std::type_identity<test_result_type>,
			son::callback_result::value_type
		)
	> make_result_mock;

	son::callback_result::value_type make_result(
		std::type_identity<son::callback_result::value_type> tag,
		test_result_type value
	)
	{ return make_result_mock(tag, value); }

	TestFwk::mock_entry<std::string_view(test_result_type)> to_string_view_mock;
	std::string_view to_string_view(test_result_type value)
	{ return to_string_view_mock(value); }

	TestFwk::mock_entry<bool(test_result_type)> is_error_mock;
	bool is_error(test_result_type type)
	{ return is_error_mock(type); }

	test_result_type make_result(
		std::type_identity<test_result_type> tag,
		son::callback_result::value_type value
	)
	{ return make_result_mock(tag, value); }
}

TESTCASE(son_callback_result_default_state)
{
	son::callback_result res;
	EXPECT_EQ(res.has_error_condition(), false);
	EXPECT_EQ(res.is_done(), false);
}

TESTCASE(son_callback_result_result_not_an_error)
{
	make_result_mock.expect_call_with_action(
		[](
			std::type_identity<son::callback_result::value_type> /*unused*/,
			test_result_type value
		){
			EXPECT_EQ(value, test_result_type::testval);
			return son::callback_result::value_type{1};
		}
	);
	is_error_mock.expect_call_with_action(
		[](test_result_type value){
			EXPECT_EQ(value, test_result_type::testval);
			return false;
		}
	);
	son::callback_result res{test_result_type::testval};
	EXPECT_EQ(res.has_error_condition(), false);
}

TESTCASE(son_callback_result_result_is_an_error)
{
	make_result_mock.expect_call_with_action(
		[](
			std::type_identity<son::callback_result::value_type> /*unused*/,
			test_result_type value
		){
			EXPECT_EQ(value, test_result_type::testval);
			return son::callback_result::value_type{1};
		}
	);
	is_error_mock.expect_call_with_action(
		[](test_result_type value){
			EXPECT_EQ(value, test_result_type::testval);
			return true;
		}
	);
	son::callback_result res{test_result_type::testval};
	EXPECT_EQ(res.has_error_condition(), true);

	make_result_mock.expect_call_with_action(
		[](
			std::type_identity<test_result_type> /*unused*/,
		 son::callback_result::value_type val
		){
			EXPECT_EQ(val, son::callback_result::value_type{1});
			return test_result_type::testval;
		}
	);
	to_string_view_mock.expect_call_with_action(
		[](test_result_type value){
			EXPECT_EQ(value, test_result_type::testval);
			return std::string_view{"Test message"};
		}
	);
	auto const msg = res.error_description();
	EXPECT_EQ(msg, "Test message");
}

TESTCASE(son_callback_result_result_zero_is_done)
{
	make_result_mock.expect_call_with_action(
		[](
			std::type_identity<son::callback_result::value_type> /*unused*/,
			test_result_type value
		){
			EXPECT_EQ(value, test_result_type::testval);
			return son::callback_result::value_type{0};
		}
	);
	is_error_mock.expect_call_with_action(
		[](test_result_type value){
			EXPECT_EQ(value, test_result_type::testval);
			return false;
		}
	);
	son::callback_result res{test_result_type::testval};
	EXPECT_EQ(res.has_error_condition(), false);
	EXPECT_EQ(res.is_done(), true);
}

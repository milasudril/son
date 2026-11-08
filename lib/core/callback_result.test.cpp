//@	{"target":{"name":"callback_result.test"}}

#include "./callback_result.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(son_callback_result_default_state)
{
	son::callback_result res;
	EXPECT_EQ(res.has_error_condition(), false);
	EXPECT_EQ(res.is_done(), false);
}

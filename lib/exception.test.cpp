//@	{"target":{"name":"exception.test"}}

#include "./exception.hpp"

#include <testfwk/testfwk.hpp>
#include <unistd.h>
#include <sys/wait.h>

TESTCASE(son_exception_what)
{
	son::exception foo{"This is a test {}", 123};
	EXPECT_EQ(foo.what(), std::string_view{"This is a test 123"});
	EXPECT_EQ((std::is_base_of_v<std::exception, son::exception>), true);
}

TESTCASE(son_raise_internal_error)
{
	TestFwk::expect_death(
		[]{
			son::raise_internal_error("Something went wrong {} {}", son::make_fmt_args(1, "Foobar"));
		},
		"jopp internal error: lib/exception.test.cpp:20: Something went wrong 1 Foobar\n",
		SIGABRT
	);
}

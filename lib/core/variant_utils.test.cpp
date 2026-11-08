//@	{"target":{"name":"./variant_utils.test"}}

#include "./variant_utils.hpp"

#include <testfwk/testfwk.hpp>
#include <testfwk/mock_util.hpp>

struct arg_type
{
	int value;
};

TESTCASE(son_variant_utils_visit_variant_element)
{
	using variant_type = std::variant<int, std::string>;
	enum class which{int_visited, string_visisted};
	TestFwk::mock_entry_overload_set<
		which(son::variant_element_tag<int>, arg_type),
		which(son::variant_element_tag<std::string>, arg_type),
		which(son::variant_element_tag<int>, int&),
		which(son::variant_element_tag<std::string>, int&)
	> visitor;

	visitor.expect_call_with_action(
		[](son::variant_element_tag<int>, arg_type arg) {
			EXPECT_EQ(arg.value, 2);
			return which::int_visited;
		}
	);
	auto const result1 = son::visit_variant_element<variant_type>(
		0,
		visitor,
		arg_type{
			.value = 2
		}
	);
	EXPECT_EQ(result1, which::int_visited);

	visitor.expect_call_with_action(
		[](son::variant_element_tag<std::string>, arg_type arg) {
			EXPECT_EQ(arg.value, 1);
			return which::string_visisted;
		}
	);
	auto const result2 = son::visit_variant_element<variant_type>(
		1,
		visitor,
		arg_type{
			.value = 1
		}
	);
	EXPECT_EQ(result2, which::string_visisted);

	int value = 1;
	visitor.expect_call_with_action(
		[](son::variant_element_tag<int>, int& arg) {
			EXPECT_EQ(arg, 1);
			arg = 2;
			return which::int_visited;
		}
	);
	auto const result3 = son::visit_variant_element<variant_type>(
		0,
		visitor,
		value
	);
	EXPECT_EQ(result3, which::int_visited);
	EXPECT_EQ(value, 2);
}

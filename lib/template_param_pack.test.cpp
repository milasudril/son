//@	{"target":{"name":"template_param_pack.test"}}

#include "./template_param_pack.hpp"

#include <variant>
#include <testfwk/testfwk.hpp>

TESTCASE(son_map_template_param_pack_to_type)
{
	using my_pack = son::template_param_pack<int, double>;

	static_assert(
		std::is_same_v<
			son::map_template_param_pack_to_type_t<std::variant, my_pack>,
			std::variant<int, double>
		>
	);
}

TESTCASE(son_wrap_in_template_param_pack)
{
	using my_pack = son::template_param_pack<int, double>;

	static_assert(std::is_same_v<son::wrap_in_template_param_pack_t<my_pack>, my_pack>);
	static_assert(std::is_same_v<son::wrap_in_template_param_pack_t<int>, son::template_param_pack<int>>);
}

TESTCASE(son_append_to_template_param_pack)
{
	using my_pack = son::template_param_pack<int, double>;

	using appended_pack = son::append_to_template_param_pack_t<my_pack, char>;

	static_assert(
		std::is_same_v<
			appended_pack,
			son::template_param_pack<int, double, char>
		>
	);
}

TESTCASE(son_concatenate_template_param_pack)
{
	using pack_a = son::template_param_pack<float, double>;
	using pack_b = son::template_param_pack<int, long long>;

	using total_pack = son::concatenate_template_param_packs_t<pack_a, pack_b>;

	static_assert(
		std::is_same_v<
			total_pack,
			son::template_param_pack<float, double, int ,long long>
		>
	);
}

namespace
{
	template<class Foo, class Bar>
	struct type_with_extra_arg
	{};

	struct test_type
	{};
}

TESTCASE(son_wrap_template_param_pack_elements)
{
	using my_pack = son::template_param_pack<float, double>;
	using my_new_pack = son::wrap_template_param_pack_elements_t<
		my_pack,
		type_with_extra_arg,
		test_type
	>;

	static_assert(
		std::is_same_v<
			my_new_pack,
			son::template_param_pack<
				type_with_extra_arg<float, test_type>,
				type_with_extra_arg<double, test_type>
			>
		>
	);
}

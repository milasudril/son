//@	{"target":{"name":"stringconv.test"}}

#include "./stringconv.hpp"

#include <testfwk/testfwk.hpp>
#include <testfwk/mock_util.hpp>

namespace
{
	struct test_string_decoder
	{
		TestFwk::mock_entry<std::string_view()> get_input_range;
		TestFwk::mock_entry<void(std::string_view)> set_input_range;

		enum class state{
			string_normal,
			string_escape,
			decode_codepoint
		};

		TestFwk::mock_entry<state()> get_state;
		TestFwk::mock_entry<void(state)> set_state;
		TestFwk::mock_entry<bool(char)> accept_char;
		TestFwk::mock_entry<son::accept_codepoint_hexdigit_result(char)> accept_codepoint_hexdigit;

		static constexpr auto string_escape_char = '\\';
		static constexpr auto string_end_char = '\"';
		static constexpr auto codepoint_begin_char = 'u';
	};
}

TESTCASE(son_stringconv_empty_returns_more_data_needed)
{
	test_string_decoder decoder;
	decoder.get_input_range.expect_call_with_action([](){return std::string_view{}; });
	auto const result = son::decode_quoted_string(decoder);
	EXPECT_EQ(result, son::string_decode_result::more_data_needed);
}

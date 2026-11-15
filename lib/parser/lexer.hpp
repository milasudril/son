#ifndef SON_LEXER_HPP
#define SON_LEXER_HPP

#include "./charskip.hpp"
#include "lib/core/operation_result.hpp"

#include <span>

namespace son
{
	template<class Lexer>
	constexpr auto run_lexer(Lexer& lexer)
	{
		while(true)
		{
			{
				auto const result = lexer.process_current_token();
				if(result.status() != operation_result_value::completed)
				{ return result; }
			}

			if(auto const result = lexer.begin_next_token())
			{ return operation_result{*result}; }
		}
	}

	class lexer
	{
	public:
		constexpr auto process_current_token()
		{ return m_current_decoder(*this); }

		constexpr void set_input_range(std::span<char const> new_range)
		{ m_input_range = new_range; }

		constexpr std::span<char const> get_input_range() const
		{ return m_input_range; }

		static constexpr bool skip_char(char ch_in)
		{ return ch_in >= 0 && ch_in <= ' '; }

		static constexpr operation_result do_charskip(lexer& lexer_state)
		{ return operation_result{skip_chars(lexer_state)}; }

	private:
		using token_decoder = operation_result (*)(lexer&);
		std::span<char const> m_input_range;
		token_decoder m_current_decoder;
	};
}

#endif

#ifndef SON_LEXER_HPP
#define SON_LEXER_HPP

#include "lib/core/operation_result.hpp"

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
}

#endif

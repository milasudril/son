#ifndef SON_LEXER_HPP
#define SON_LEXER_HPP

#include "./charskip.hpp"
#include "lib/core/exception.hpp"
#include "lib/core/int_types.hpp"
#include "lib/core/operation_result.hpp"

#include <span>
#include <optional>

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

	enum class lexer_begin_next_token_result:half_uintptr_t
	{
		lexing_completed
	};

	constexpr char const* to_string(lexer_begin_next_token_result res)
	{
		switch(res)
		{
			case lexer_begin_next_token_result::lexing_completed:
				return "Lexing completed";
			default:
				return "Unknown error";
		}
	}

	constexpr auto convert_to(
		std::type_identity<operation_result_value> /*unused*/,
		lexer_begin_next_token_result res
	)
	{
		switch(res)
		{
			case lexer_begin_next_token_result::lexing_completed:
				return operation_result_value::completed;
			default:
				return operation_result_value::error;
		}
	}

	enum class update_stream_state_result:half_uintptr_t{
		lexing_completed,
		lexing_aborted
	};

	constexpr char const* to_string(update_stream_state_result value)
	{
		switch(value)
		{
			case update_stream_state_result::lexing_completed:
				return "Lexing completed";
			case update_stream_state_result::lexing_aborted:
				return "Lexing aborted";
		}
		return nullptr;
	}

	constexpr auto convert_to(
		std::type_identity<operation_result_value> /*unused*/,
		update_stream_state_result value
	)
	{
		switch(value)
		{
			case update_stream_state_result::lexing_completed:
				return operation_result_value::completed;
			case update_stream_state_result::lexing_aborted:
				return operation_result_value::aborted;
		}
		raise_internal_error("Invalid update_stream_state_result");
	}

	class lexer
	{
	public:
		constexpr auto process_current_token()
		{ return m_current_decoder(*this); }

		static constexpr std::optional<lexer_begin_next_token_result> begin_next_token()
		{ return lexer_begin_next_token_result::lexing_completed; }

		static constexpr std::optional<update_stream_state_result> update_stream_state(char ch_in)
		{
			switch(ch_in)
			{
				case '\0':
					return update_stream_state_result::lexing_completed;
				case '\x18':
					return update_stream_state_result::lexing_aborted;
				default:
					return std::nullopt;
			}
		}

		constexpr void set_input_range(std::span<char const> new_range)
		{ m_input_range = new_range; }

		constexpr std::span<char const> get_input_range() const
		{ return m_input_range; }

		static constexpr bool skip_char(char ch_in)
		{ return ch_in >= 0 && ch_in <= ' '; }

		static constexpr operation_result do_charskip(lexer& lexer)
		{ return skip_chars(lexer); }

	private:
		using token_decoder = operation_result (*)(lexer&);
		std::span<char const> m_input_range;
		token_decoder m_current_decoder;
	};
}

#endif

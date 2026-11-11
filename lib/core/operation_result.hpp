#ifndef SON_OPERATION_RESULT_HPP
#define SON_OPERATION_RESULT_HPP

#include "./int_types.hpp"

namespace son
{
	enum class operation_result_value:half_uintptr_t{
		done,
		suspended,
		error
	};

	class operation_result
	{
	public:
		template<class Result>
		requires(
			std::is_enum_v<Result> &&
			std::is_same_v<std::underlying_type_t<Result>, half_uintptr_t>
		)
		explicit operation_result(Result res):
			m_status{convert_to(std::type_identity<operation_result_value>{}, res)},
			m_orig_value{static_cast<half_uintptr_t>(res)},
			m_error_description{
				[](half_uintptr_t res) noexcept {
					return to_string(static_cast<Result>(res));
				}
			}
		{}

		constexpr char const* error_description() const noexcept
		{ return m_error_description(m_orig_value); }

		constexpr auto status() const
		{ return m_status; }

	private:
		operation_result_value m_status{};
		half_uintptr_t m_orig_value{};
		char const* (*m_error_description)(half_uintptr_t) noexcept {};
	};
}

#endif

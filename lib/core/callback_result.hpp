#ifndef SON_CALLBACK_RESULT_HPP
#define SON_CALLBACK_RESULT_HPP

#include "./int_types.hpp"
#include "./exception.hpp"

namespace son
{
	class callback_result
	{
	public:
		enum class status_type:half_uintptr_t{
			done,
			more_data_needed,
			error
		};

		callback_result() = default;

		template<class Result>
		requires(
			std::is_enum_v<Result> &&
			std::is_same_v<std::underlying_type_t<Result>, half_uintptr_t>
		)
		explicit callback_result(Result res):
			m_status{convert_to(std::type_identity<status_type>{}, res)},
			m_orig_value{static_cast<half_uintptr_t>(res)},
			m_error_description{
				[](half_uintptr_t res) noexcept {
					return to_string(static_cast<Result>(res));
				}
			}
		{}

		constexpr char const* error_description() const noexcept
		{
			if(m_error_description == nullptr)
			{ raise_internal_error("callback_result has no error message"); }
			return m_error_description(m_orig_value);
		}

		constexpr auto status() const
		{ return m_status; }

	private:
		status_type m_status{};
		half_uintptr_t m_orig_value{};
		char const* (*m_error_description)(half_uintptr_t) noexcept {};
	};
}

#endif

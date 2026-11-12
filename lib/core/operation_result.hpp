#ifndef SON_OPERATION_RESULT_HPP
#define SON_OPERATION_RESULT_HPP

#include "./int_types.hpp"

namespace son
{
	/**
	 * @brief Represents the fundamental status values of an operation.
	 */
	enum class operation_result_value : half_uintptr_t {
		/**
		 * \brief Operation completed successfully
		 */
		done,

		/**
		 * \brief Operation is currently suspended
		 */
		suspended,

		/**
		 * \brief Operation encountered an error
		 */
		error
	};

	/**
	 * @brief Converts an operation result value using type identity for overload resolution.
	 *
	 * @param ... Unused type identity parameter.
	 * @param res The operation result value to convert.
	 * @return constexpr auto The converted operation result value.
	 */
	constexpr auto convert_to(
		std::type_identity<operation_result_value> /*unused*/,
		operation_result_value res
	)
	{ return res; }

	/**
	 * @brief Converts an operation result value to its corresponding string description.
	 *
	 * @param value The operation result value to inspect.
	 * @return constexpr const char* A null-terminated string description of the status.
	 */
	constexpr const char* to_string(operation_result_value value)
	{
		switch(value)
		{
			case operation_result_value::done:
				return "Operation completed";
			case operation_result_value::suspended:
				return "Operation suspended";
			case son::operation_result_value::error:
			default:
				return "Unknown error";
		}
	}

	/**
	 * @brief Encapsulates the result of an operation, supporting generic enum result types
	 *		that share a compatible underlying type (`half_uintptr_t`).
	 */
	class operation_result
	{
	public:
		/**
		 * @brief Constructs an operation_result from a custom enum result type.
		 *
		 * @tparam Result An enum type whose underlying type is `half_uintptr_t`.
		 * @param res The result value to wrap and store.
		 */
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

		/**
		 * @brief Retrieves the error or status description for the stored result.
		 *
		 * @return constexpr char const* A null-terminated string describing the result.
		 */
		constexpr char const* error_description() const noexcept
		{ return m_error_description(m_orig_value); }

		/**
		 * @brief Retrieves the normalized operation status value.
		 *
		 * @return constexpr auto The status as an `operation_result_value`.
		 */
		constexpr auto status() const
		{ return m_status; }

	private:
		operation_result_value m_status{};
		half_uintptr_t m_orig_value{};
		char const* (*m_error_description)(half_uintptr_t) noexcept {};
	};
}

#endif

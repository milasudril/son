#ifndef SON_CALLBACK_RESULT_HPP
#define SON_CALLBACK_RESULT_HPP

#include <string_view>

namespace son
{
	class callback_result
	{
	public:
		static constexpr size_t operation_completed = 0;

		struct value_type
		{
			size_t value;
			constexpr bool operator==(value_type const&) const = default;
			constexpr bool operator!=(value_type const&) const = default;
		};

		callback_result() = default;

		template<class Result>
		explicit callback_result(Result res):
			m_value{make_result(std::type_identity<callback_result::value_type>{}, res)},
			m_error_description{
				is_error(res)?
					[](value_type res) noexcept {
						return to_string_view(make_result(std::type_identity<Result>{}, res));
					}:
					nullptr
			}
		{}

		constexpr bool is_done() const
		{ return m_value == value_type{operation_completed};}

		constexpr bool has_error_condition() const
		{ return m_error_description != nullptr; }

		constexpr std::string_view error_description() const noexcept
		{ return m_error_description(m_value); }

		constexpr size_t value() const
		{ return m_value.value; }

	private:
		value_type m_value{1};
		std::string_view (*m_error_description)(value_type) noexcept {};
	};
}

#endif

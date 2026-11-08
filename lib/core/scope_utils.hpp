#ifndef SON_SCOPE_UTILS_HPP
#define SON_SCOPE_UTILS_HPP

#include <type_traits>
#include <utility>
#include <optional>

namespace son
{
	template<class Callable>
	class scope_exit
	{
	public:
		template<class T>
		requires(!std::is_same_v<std::remove_cvref_t<T>, scope_exit>)
		explicit scope_exit(T&& cb):
			m_cb{std::in_place_t{}, std::forward<T>(cb)}
		{}

		scope_exit(scope_exit const&) = delete;
		scope_exit(scope_exit&&) = delete;
		scope_exit& operator=(scope_exit const&) = delete;
		scope_exit& operator=(scope_exit&&) = delete;

		void release()
		{ m_cb.reset(); }

		~scope_exit()
		{
			if(m_cb)
			{ (*m_cb)(); }
		}

	private:
		std::optional<Callable> m_cb;
	};

	template<class T>
	constexpr auto make_scope_exit(T&& callable)
	{ return scope_exit<std::remove_cvref_t<T>>{std::forward<T>(callable)}; }
};

#endif

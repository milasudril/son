#ifndef SON_NAMED_OBJECT_HPP
#define SON_NAMED_OBJECT_HPP

#include <utility>

namespace son
{
	template<class NameType, class Base>
	class named_object:public Base
	{
	public:
		using name_type = NameType;

		using Base::Base;

		template<class ... Args>
		constexpr explicit named_object(name_type&& subtype_id, Args&&... args):
			Base{std::forward<Args>(args)...},
			m_subtype_id{std::move(subtype_id)}
		{}

		constexpr name_type const& subtype_id() const
		{ return m_subtype_id;}

	private:
		name_type m_subtype_id;
	};
}

#endif

//@	{"target":{"name":"clear_nodes_visitor.test"}}

#include "./clear_nodes_visitor.hpp"
#include "lib/node_visitor_adaptor.hpp"

#include <testfwk/testfwk.hpp>
#include <testfwk/mock_util.hpp>

namespace
{
	struct thing_with_clear
	{
		TestFwk::mock_entry<void()> clear_backing_store;
	};
}

TESTCASE(son_clear_nodes_visitor_handle_leaf_value)
{
	son::clear_nodes_visitor visitor;
	EXPECT_EQ(
		visitor.handle_leaf_value(123, son::value_visitation_context{}),
		son::node_visitor_status::ready
	)
}

TESTCASE(son_clear_nodes_visitor_handle_simple_array)
{
	son::clear_nodes_visitor visitor;
	thing_with_clear testobj;
	testobj.clear_backing_store.expect_call_with_action([]{});
	EXPECT_EQ(
		visitor.handle_simple_array(testobj, son::value_visitation_context{}),
		son::node_visitor_status::ready
	)
}

TESTCASE(son_clear_nodes_visitor_handle_begin_of_container)
{
	son::clear_nodes_visitor visitor;
	thing_with_clear testobj;;
	EXPECT_EQ(
		visitor.handle_begin_of_container(testobj, son::value_visitation_context{}),
		son::node_visitor_status::ready
	)
}

TESTCASE(son_clear_nodes_visitor_handle_end_of_container)
{
	son::clear_nodes_visitor visitor;
	thing_with_clear testobj;
	testobj.clear_backing_store.expect_call_with_action([]{});
	EXPECT_EQ(
		visitor.handle_end_of_container(testobj, son::value_visitation_context{}),
		son::node_visitor_status::ready
	)
}

TESTCASE(son_clear_nodes_visitor_handle_key)
{
	son::clear_nodes_visitor visitor;
	EXPECT_EQ(
		visitor.handle_key(123, son::value_visitation_context{}),
		son::node_visitor_status::ready
	)
}

TESTCASE(son_clear_nodes_visitor_handle_subtype_id)
{
	son::clear_nodes_visitor visitor;
	EXPECT_EQ(
		visitor.handle_subtype_id(123, son::value_visitation_context{}),
		son::node_visitor_status::ready
	)
}

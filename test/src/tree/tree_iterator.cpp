/**
 * @brief Testing Tree Iterators.
 *
 * @file
 * @ingroup test
 */

#include "common.hpp"

#include <string>
#include <utility>

#include "lib/tree/default_tree_newick_adapter.hpp"
#include "lib/tree/newick_processor.hpp"
#include "lib/tree/tree.hpp"

using namespace genesis;

// =================================================================================================
//     Eulertour
// =================================================================================================

/*
class Eulertour : public ::testing::TestWithParam< std::pair<std::string, std::string> >
{};

TEST_P (Eulertour, Test)
{
    std::string input = "((B,(D,E)C)A,F,(H,I)G)R;";
    std::string nodes = "";

    DefaultTree tree;
    DefaultTreeNewickProcessor().from_string(input, tree);

    auto node = tree.find_node(GetParam().first);
    ASSERT_NE(nullptr, node);

    for (auto it = tree.begin_eulertour(node); it != tree.end_eulertour(); ++it) {
        nodes += it.node()->name;
    }
    EXPECT_EQ(GetParam().second, nodes) << " with start node " << GetParam().first;
}

INSTANTIATE_TEST_CASE_P (TreeIterator, Eulertour, ::testing::Values(
    std::make_pair("R", "RABACDCECARFRGHGIG"),
    std::make_pair("A", "ARFRGHGIGRABACDCEC"),
    std::make_pair("B", "BACDCECARFRGHGIGRA"),
    std::make_pair("C", "CARFRGHGIGRABACDCE"),
    std::make_pair("D", "DCECARFRGHGIGRABAC"),
    std::make_pair("E", "ECARFRGHGIGRABACDC"),
    std::make_pair("F", "FRGHGIGRABACDCECAR"),
    std::make_pair("G", "GRABACDCECARFRGHGI"),
    std::make_pair("H", "HGIGRABACDCECARFRG"),
    std::make_pair("I", "IGRABACDCECARFRGHG")
));
*/

void TestEulertour(std::string node_name, std::string out_nodes)
{
    std::string input = "((B,(D,E)C)A,F,(H,I)G)R;";
    std::string nodes = "";

    DefaultTree tree;
    DefaultTreeNewickProcessor().from_string(input, tree);

    auto node = tree.find_node(node_name);
    ASSERT_NE(nullptr, node);

    for (auto it = tree.begin_eulertour(node); it != tree.end_eulertour(); ++it) {
        nodes += it.node()->name;
    }
    EXPECT_EQ(out_nodes, nodes) << " with start node " << node_name;
}

TEST (TreeIterator, Eulertour)
{
    TestEulertour("R", "RABACDCECARFRGHGIG");
    TestEulertour("A", "ARFRGHGIGRABACDCEC");
    TestEulertour("B", "BACDCECARFRGHGIGRA");
    TestEulertour("C", "CARFRGHGIGRABACDCE");
    TestEulertour("D", "DCECARFRGHGIGRABAC");
    TestEulertour("E", "ECARFRGHGIGRABACDC");
    TestEulertour("F", "FRGHGIGRABACDCECAR");
    TestEulertour("G", "GRABACDCECARFRGHGI");
    TestEulertour("H", "HGIGRABACDCECARFRG");
    TestEulertour("I", "IGRABACDCECARFRGHG");
}

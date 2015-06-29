/**
 * @brief Implementation of functions for reading and writing Phyloxml files.
 *
 * For reasons of readability, in this implementation file, the template data types
 * NodeDataType and EdgeDataType are abbreviated using NDT and EDT, respectively.
 *
 * @file
 * @ingroup tree
 */

#include <assert.h>
#include <vector>

#include "tree/tree.hpp"
#include "utils/logging.hpp"
#include "utils/utils.hpp"
#include "utils/xml_document.hpp"
#include "utils/xml_processor.hpp"

namespace genesis {

// =================================================================================================
//     Parsing
// =================================================================================================

// =================================================================================================
//     Printing
// =================================================================================================

/**
 * @brief Writes the tree to a file in Phyloxml format.
 *
 * If the file already exists, the function does not overwrite it.
 */
template <class TreeType>
bool PhyloxmlProcessor::to_file (const std::string fn, const TreeType& tree)
{
    if (file_exists(fn)) {
        LOG_WARN << "Phyloxml file '" << fn << "' already exist. Will not overwrite it.";
        return false;
    }
    std::string ts;
    to_string(ts, tree);
    return file_write(fn, ts);
}

/**
 * @brief Gives a Phyloxml string representation of the tree.
 *
 * In case the tree was read from a Phyloxml file, this function should produce the same
 * representation.
 */
template <class TreeType>
void PhyloxmlProcessor::to_string (std::string& ts, const TreeType& tree)
{
    ts = to_string(tree);
}

/**
 * @brief Returns a Phyloxml string representation of the tree.
 *
 * In case the tree was read from a Phyloxml file, this function should produce the same
 * representation.
 */
template <class TreeType>
std::string PhyloxmlProcessor::to_string (const TreeType& tree)
{
    XmlDocument xml;
    to_document(xml, tree);
    return XmlProcessor().to_string(xml);
}

/**
 * @brief Stores the information of the tree into an Phyloxml-formatted XmlDocument.
 */
template <class TreeType>
void PhyloxmlProcessor::to_document (XmlDocument& xml, const TreeType& tree)
{
    xml.clear();

    // Set XML declaration.
    xml.xml_tag = "xml";
    xml.declarations.emplace("version",  "1.0");
    xml.declarations.emplace("encoding", "UTF-8");

    // Set XML root element.
    xml.tag = "Phyloxml";
    xml.attributes.emplace("xmlns:xsi",          "http://www.w3.org/2001/XMLSchema-instance");
    xml.attributes.emplace("xsi:schemaLocation", "http://www.Phyloxml.org http://www.Phyloxml.org/1.10/Phyloxml.xsd");
    xml.attributes.emplace("xmlns",              "http://www.Phyloxml.org");

    // Add the (phylogeny) element.
    auto phylogeny = make_unique<XmlElement>();
    phylogeny->tag = "phylogeny";
    phylogeny->attributes.emplace("rooted",     "true");
    //~ phylogeny.attributes.emplace("rerootable", "true");

    // Create a stack where we will push the tree elements to. Use the phylogeny element as root.
    std::vector<XmlElement*> stack;
    stack.push_back(phylogeny.get());
    xml.content.push_back(std::move(phylogeny));
    int cur_d = 0;

    // Store the distance from each node to the root. Will be used to determine the position on the
    // stack that is used for adding clades to the phylogeny.
    std::vector<int> depths = tree.node_depth_vector();

    for (
        typename TreeType::ConstIteratorPreorder it = tree.begin_preorder();
        it != tree.end_preorder();
        ++it
    ) {
        // Depth can never increase more than one between two nodes when doing a preoder traversal.
        assert(depths[it.node()->index()] <= cur_d + 1);

        // Delete end of stack when moving up the tree, unless we are already at the root.
        while (cur_d >= depths[it.node()->index()] && depths[it.node()->index()] > 0) {
            assert(stack.size() > 0);
            stack.pop_back();
            --cur_d;
        }
        // Set current depth (explicitly needed in case we are moving further into the tree, which
        // means that the loop above is not executed).
        cur_d = depths[it.node()->index()];

        // Create clade element, append it to the stack, so that all sub-elements will use it as
        // parent.
        auto clade = make_unique<XmlElement>();
        clade->tag = "clade";

        // create name for clade.
        // TODO move to node.toPhyloxmlelement
        auto name_e = make_unique<XmlElement>("name");
        auto name_m = make_unique<XmlMarkup>(it.node()->name);
        name_e->content.push_back(std::move(name_m));
        clade->content.push_back(std::move(name_e));

        //~ it.node()->to_newick_broker_element(bn);
        // only write edge data to the broker element if it is not the last iteration.
        // the last iteration is the root, which usually does not have edge information in newick.
        // caveat: for the root node, the edge will point to an arbitrary edge away from the root.
        //~ if (!it.is_last_iteration()) {
            //~ it.edge()->to_newick_broker_element(bn);
        //~ }

        // filter out default names if needed
        //~ if (!use_default_names && bn->name != "" && (
            //~ bn->name == default_leaf_name ||
            //~ bn->name == default_internal_name ||
            //~ bn->name == default_root_name
        //~ )) {
            //~ bn->name = "";
        //~ }

        // Append the clade to the current parent (end of the stack), then use it as the new parent
        // for the next iteration of the loop.
        auto clade_ptr = clade.get();
        stack.back()->content.push_back(std::move(clade));
        stack.push_back(clade_ptr);
    }
}

} // namespace genesis

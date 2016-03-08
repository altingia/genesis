#ifndef GENESIS_PLACEMENT_IO_NEWICK_PROCESSOR_H_
#define GENESIS_PLACEMENT_IO_NEWICK_PROCESSOR_H_

/**
 * @brief Header of Placement Tree Newick Processor class.
 *
 * @file
 * @ingroup placement
 */

#include <assert.h>
#include <stdexcept>
#include <string>

#include "placement/placement_tree.hpp"
#include "tree/default/newick_mixin.hpp"
#include "tree/io/newick/processor.hpp"

namespace genesis {
namespace placement {

// =================================================================================================
//     Placement Tree Newick Mixin
// =================================================================================================

/**
 * @brief
 */
template <typename Base>
class PlacementTreeNewickMixin : public Base
{
    // -------------------------------------------------------------------------
    //     Member Types
    // -------------------------------------------------------------------------

public:

    typedef typename Base::TreeType TreeType;
    typedef typename Base::NodeType NodeType;
    typedef typename Base::EdgeType EdgeType;
    typedef typename Base::LinkType LinkType;

    // -------------------------------------------------------------------------
    //     Properties
    // -------------------------------------------------------------------------

public:

    bool enable_edge_nums() const
    {
        return enable_edge_nums_;
    }

    void enable_edge_nums(bool value)
    {
        enable_edge_nums_ = value;
    }

    bool enable_placement_counts() const
    {
        return enable_placement_counts_;
    }

    void enable_placement_counts(bool value)
    {
        enable_placement_counts_ = value;
    }

    // -------------------------------------------------------------------------
    //     Overridden Member Functions
    // -------------------------------------------------------------------------

protected:

    virtual void element_to_edge( NewickBrokerElement const& element, EdgeType& edge ) override
    {
        Base::element_to_edge(element, edge);

        // Process the edge num.
        edge.data.edge_num      = -1;
        if (element.tags.size() == 0) {
            throw std::invalid_argument(
                "Edge at node '" + element.name + "' does not contain a tag value like '{42}'" +
                " for the placement edge_num of this edge."
            );
        }
        if (element.tags.size() > 1) {
            throw std::invalid_argument(
                "Edge at node '" + element.name + "' contains more than one tag value like " +
                "'{xyz}'. Expecting only one for the placement edge_num of this edge."
            );
        }
        assert(element.tags.size() == 1);
        edge.data.edge_num = std::stoi(element.tags[0]);
    }

    virtual void edge_to_element( EdgeType const& edge, NewickBrokerElement& element ) override
    {
        Base::edge_to_element(edge, element);

        if (enable_edge_nums_) {
            element.tags.push_back(std::to_string(edge.data.edge_num));
        }
        if (enable_placement_counts_) {
            element.comments.push_back(std::to_string(edge.data.placement_count()));
        }
    }

    // -------------------------------------------------------------------------
    //     Data Members
    // -------------------------------------------------------------------------

private:

    bool enable_edge_nums_        = true;
    bool enable_placement_counts_ = false;
};

// =================================================================================================
//     Placement Tree Newick Processor
// =================================================================================================

typedef PlacementTreeNewickMixin< DefaultTreeNewickMixin< NewickProcessor< PlacementTree > > >
    PlacementTreeNewickProcessor;

} // namespace placement
} // namespace genesis

#endif // include guard

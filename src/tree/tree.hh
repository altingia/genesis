#ifndef GNS_TREE_TREE_H_
#define GNS_TREE_TREE_H_

/**
 * @brief Provides the basic interface for phylogenetic trees.
 *
 * For more information, see Tree class.
 *
 * @file
 * @ingroup tree
 */

#include <string>
#include <vector>

#include "tree/tree_branch.hh"
#include "tree/tree_broker.hh"
#include "tree/tree_link.hh"
#include "tree/tree_node.hh"

namespace genesis {

class Tree
{
public:
    Tree () {}
    void clear();
    virtual ~Tree();

    bool FromNewickFile (const std::string& fn);
    bool FromNewickString (const std::string& tree);
    void FromTreeBroker (TreeBroker& broker);

    void        ToNewickFile   (const std::string& fn);
    std::string ToNewickString ();
    void        ToTreeBroker   (TreeBroker& broker);

    std::string DumpBranches() const;
    std::string DumpLinks() const;
    std::string DumpNodes() const;
    std::string DumpRoundtrip() const;

    int MaxRank();

    inline bool IsBifurcating()
    {
        return MaxRank() == 2;
    }

protected:
    int BranchPointerToIndex (TreeBranch* branch) const;
    int LinkPointerToIndex   (TreeLink*   link)   const;
    int NodePointerToIndex   (TreeNode*   node)   const;

    std::vector<TreeLink*>   links_;
    std::vector<TreeNode*>   nodes_;
    std::vector<TreeBranch*> branches_;
};

} // namespace genesis

#endif // include guard

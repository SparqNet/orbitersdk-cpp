#ifndef MERKLE_H
#define MERKLE_H

#include <string>
#include <vector>
#include "strings.h"

/**
 * Custom implementation of a Merkle tree.
 * Adapted from:
 * https://medium.com/coinmonks/implementing-merkle-tree-and-patricia-tree-b8badd6d9591
 * https://lab.miguelmota.com/merkletreejs/example/
 */
class Merkle {
  private:
    std::vector<std::vector<Hash>> tree;  ///< The Merkle tree itself.

    /**
     * Insert a new layer in the Merkle tree.
     * @param layer The list of hashes to convert into a layer.
     * @return The newly created layer.
     */
    std::vector<Hash> newLayer(const std::vector<Hash> layer);
  public:
    /**
     * Constructor.
     * @param leaves The list of leaves to create the Merkle tree from.
     */
    Merkle(const std::vector<Hash> leaves);

    /**
     * Constructor.
     * @param txs The list of transactions to create the Merkle tree from.
     */
    Merkle(const std::unordered_map<uint64_t, Tx, SafeHash> txs);

    /// Get the root of the Merkle tree.
    inline const Hash getRoot() { return this->tree.back().front(); }

    /// Getter for `tree`.
    inline const std::vector<std::vector<Hash>> getTree() { return this->tree; }

    /**
     * Get the proof for a given leaf in the Merkle tree.
     * @param leafIndex The index of the leaf.
     *                  Considers only the leaf layer (e.g. {A, B, C, D, E, F},
     *                  `getProof(2)` would get the proof for leaf C).
     * @return A list of proofs for the leaf.
     */
    const std::vector<Hash> getProof(const uint64_t leafIndex);
};

/**
 * Abstraction of a Patricia tree node.
 * Used internally by the %Patricia class.
 */
class PNode {
  private:
    char id;  ///< ID of the node. `/` implies it's the root node.
    std::string data; ///< Data of the node. Non-empty implies it's the end node.
    std::vector<PNode> children;  ///< List of children nodes.
  public:
    /**
     * Constructor.
     * @param id The ID of the node.
     */
    PNode(char id) : id(id) {};

    /// Getter for `id`.
    inline char getId() { return this->id; }

    /// Getter for `data`.
    inline std::string getData() { return this->data; }

    /// Setter for `data.
    inline void setData(std::string data) { this->data = data; }

    /**
     * Check if the node has any children.
     * @returns `true` if the node has children, `false` otherwise.
     */
    inline bool hasChildren() { return this->children.size() > 0; }

    /**
     * Add a child to the node.
     * @param id The ID of the child node.
     */
    inline void addChild(char id) { this->children.emplace_back(PNode(id)); }

    /**
     * Get a child from the node.
     * @param id The ID of the child node.
     * @return A pointer to the child node, or a NULL pointer if not found.
     */
    PNode* getChild(char id);
};

/**
 * Custom implementation of a Patricia tree.
 * Adapted from:
 * https://medium.com/coinmonks/implementing-merkle-tree-and-patricia-tree-b8badd6d9591
 * https://lab.miguelmota.com/merkletreejs/example/
 */
class Patricia {
  private:
    PNode root; ///< Root node.
  public:
    Patricia() : root('/') {}; ///< Constructor. Sets the ID of the root node to `/`.

    /**
     * Create a new branch in the tree and add a leaf node with data.
     * @param branch The hash string to use as a base for creating the branch.
     * @param data The data string to add to the leaf node.
     */
    void addLeaf(Hash branch, std::string data);

    /**
     * Get data from a leaf node in a given branch.
     * @param branch The hash string to use as a base for searching the branch.
     * @return The data string contained in the leaf node.
     */
    std::string getLeaf(Hash branch);

    /**
     * Remove data from a leaf node in a given branch.
     * Does not remove the branch itself.
     * @param branch The hash string to use as a base for removing data from the branch.
     * @return `true` if the removal was successful, `false` otherwise.
     */
    bool delLeaf(Hash branch);
};

#endif  // MERKLE_H
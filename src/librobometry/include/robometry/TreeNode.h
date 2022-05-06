/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef ROBOMETRY_TREE_NODE_H
#define ROBOMETRY_TREE_NODE_H

#include <cstddef>
#include <unordered_map>
#include <memory>
#include <string>
#include <iostream>
#include <regex>
#include <assert.h>

#include <matioCpp/Span.h>

namespace robometry {

/**
 * @brief A class to represent the Node in Tree struct
 *
 */
template<class T>
class TreeNode
{
public:
    /**
     * @brief Construct an empty Node of tree.
     */
    TreeNode () = default;

    /**
     * @brief Construct an empty Node of tree containing a value.
     *
     * @param[in] _value An already initialized pointer containing the value stored in the nore
     */
    TreeNode(std::shared_ptr<T> _value)
        : m_value(_value) {
        assert(m_value);
    }

    /**
     * @brief Check if a child with a given name exist
     *
     * @param[in] name The name of the child
     * @return True if the child exist false otherwise.
     */
    [[nodiscard]]
    bool childExists(const std::string& name) const {
        return m_children.find(name) != m_children.end();
    }

    /**
     * @brief Add a new node in the tree. This node will be the child of the this node
     *
     * @param[in] name The name of the child
     * @param[in] node A pointer to the node
     * @return True if the child has been added, false otherwise.
     */
    bool addChild(const std::string& name, std::shared_ptr<TreeNode<T>> node) {

        if(this->childExists(name)) {
            std::cout << "The TreeNode named " << name << " already exist." << std::endl;
            return false;
        }

        if (node == nullptr) {
            std::cout << "The node should point to an already initialized memory." << std::endl;
            return false;
        }

        // insert the new e element in the map
        const auto out = m_children.insert({name, node});
        return out.second;
    }

    /**
     * @brief Get a child from the node.
     *
     * @param[in] name The name of the child.
     * @return A pointer to TreeNode, if the child is not found the weak pointer cannot be locked.
     */
    std::weak_ptr<TreeNode<T>> getChild(const std::string& name) const {
        if(this->childExists(name)) {
            return m_children.at(name);
        }

        return std::make_shared<TreeNode<T>>();
    }

    /**
     * @brief Get the value stored in the node.
     *
     * @return A pointer to the value stored in the node.
     */
    std::shared_ptr<T> getValue() {
        return m_value;
    }

    /**
     * @brief Get the map representing all the children associated to the node.
     *
     * @return The map of the children
     */
    const std::unordered_map<std::string, std::shared_ptr<TreeNode>>& getChildren() const {
        return m_children;
    }


    /**
     * @brief Return a standard text representation of the content of the node.
     *
     * @return a string containing the standard text representation of the content of the object.
     */
    [[nodiscard]]
    std::string toString(const std::string& name= ".", const unsigned int depth = 0) const {
        std::ostringstream oss;
        for (unsigned int i = 0 ; i < depth ; i++) {
            if (i != depth-1) {
                oss <<  "    ";
            } else {
                oss <<  "|-- ";
            }
        }
        oss  << name << std::endl;
        for (const auto & [key, child] : m_children) {
            oss << child->toString(key, depth + 1);
        }

        return oss.str();
    }

    /**
     * @brief Check if the node is empty
     *
     * @return true if a node is empty
     */
    [[nodiscard]]
    bool empty() const {
        return m_children.empty() && m_value == nullptr;
    }

    /**
     * Split a string using the separator
     *
     * @param[in] input the stribg that should be split
     * @return an std::vector containing the substrings
     */
    static std::vector<std::string> splitString(const std::string& input) {
        std::regex re(stringSeparator);
        std::sregex_token_iterator first{input.begin(), input.end(), re, -1}, last;
        return {first, last};
    };

    static std::string stringSeparator; /**< The string separator the default value is :: */

private:

    std::shared_ptr<T> m_value;
    std::unordered_map<std::string, std::shared_ptr<TreeNode>> m_children;
};

template<class T>
std::string TreeNode<T>::stringSeparator = "::";


/**
 * @brief Add a new leaf in the tree
 *
 * @param[in] nodes A span representing a list of nodes
 * @param[in] element The value stored by the leaf
 * @param[in] treeNode a pointer to a tree node
 * @return True in case of success false otherwise
 */
template<typename T>
bool addLeaf(matioCpp::Span<const std::string> nodes,
             std::shared_ptr<T> element,
             std::shared_ptr<TreeNode<T>> treeNode) {

    assert(nodes.begin() != nodes.end());

    // get the first node in the in nodes
    const std::string node = *nodes.begin();

    // if the size of the node is equal to 1 means that this is a leaf and so it will contain
    // the element
    if(nodes.size() == 1) {
        if(!treeNode->addChild(node, std::make_shared<TreeNode<T>>(element))) {
            return false;
        }
        return true;
    }

    // create a new child
    if (!treeNode->childExists(node)) {
        if (!treeNode->addChild(node, std::make_shared<TreeNode<T>>())) {
            return false;
        }
    }

    // this is always true since the child has been just added
    assert(treeNode->getChild(node).lock() != nullptr);

    // propagate the leaf creation to the child
    return addLeaf(nodes.subspan(1), element, treeNode->getChild(node).lock());
}

/**
* @brief Add a new leaf in the tree
*
* @param[in] name A string containing the address of the leaf. Use the TreeNode<T>::stringSeparator
* to define multiple nodes
* @param[in] element The content of the leaf
* @param[in] treeNode a pointer to a tree node
* @return True in case of success false otherwise
*/
template<typename  T>
bool addLeaf(const std::string& name,
             std::shared_ptr<T> element,
             std::shared_ptr<TreeNode<T>> treeNode) {
    // split the string using the separator
    const std::vector<std::string> nodes = TreeNode<T>::splitString(name);

    // build the leaf
    return addLeaf(nodes, element, treeNode);
}

/**
* @brief Get the leaf from a node.
*
* @param[in] nodes A span representing a list of nodes
* @param[in] treeNode a pointer to a tree node
* @return A pointer to TreeNode, if the child is not found the weak pointer cannot be locked.
*/
template<typename  T>
std::weak_ptr<TreeNode<T>> getLeaf(matioCpp::Span<const std::string> nodes,
                                   std::shared_ptr<TreeNode<T>> treeNode) {
    if (treeNode == nullptr) {
        return std::make_shared<TreeNode<T>>();
    }

    // check that the nodes is not an empty span
    assert(nodes.begin() != nodes.end());

    // get the name of the node
    const std::string nodeName = *nodes.begin();

    // Try to find the child named nodeName if not found a not lockable weak_ptr is returned
    auto ptr = treeNode->getChild(nodeName).lock();
    if (ptr == nullptr) {
        return std::make_shared<TreeNode<T>>();
    }

    // if the size of the nodes is equal to one the child is actually the leaf
    if (nodes.size() == 1) {
        return ptr;
    }

    return getLeaf(nodes.subspan(1), ptr);
}

/**
* @brief Get the leaf from a node.
*
* @param[in] name A string containing the address of the leaf. Use the TreeNode<T>::stringSeparator
* to define multiple nodes
* @param[in] treeNode a pointer to a tree node
* @return A pointer to TreeNode, if the child is not found the weak pointer cannot be locked.
*/
template<typename  T>
std::weak_ptr<TreeNode<T>> getLeaf(const std::string& name,
                                   std::shared_ptr<TreeNode<T>> treeNode) {
    const std::vector<std::string> nodes = TreeNode<T>::splitString(name);
    return getLeaf(nodes, treeNode);
}

} // robometry

#endif

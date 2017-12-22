/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016 Regents of the University of California.
 *
 * This file is part of Consumer/Producer API library.
 *
 * Consumer/Producer API library library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Consumer/Producer API library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with Consumer/Producer API, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of Consumer/Producer API authors and contributors.
 */

#ifndef TREE_NODE_HPP
#define TREE_NODE_HPP

#include "common.hpp"

namespace ndn {

class TreeNode
{
private:
  Name name;
  std::vector<TreeNode*> children;

  bool isMarked;
  TreeNode* parent;
  bool dataNode;

  uint64_t revisionCount;
  uint64_t treeSize;

private:
  /**
   * Helper function for the constructor to initialize the node.
   */
  void
  init(Name& name, TreeNode* parent);

public:
  /**
   * Constructor for nodes.
   */
  TreeNode(Name& name, TreeNode* parent);

  TreeNode(const TreeNode& other);

  TreeNode();

  /**
   * Function to get the name associated with the TreeNode.
   */
  Name
  getName();

  /**
   * Function to get the children of the TreeNode.
   */
  std::vector<TreeNode*>
  getChildren();

  /**
   * Function to mark the TreeNode.
   */
  bool
  markNode(bool status);

  /**
   * Function to check if the TreeNode is marked.
   */
  bool
  isNodeMarked();

  /**
   * Function to check if the TreeNode has data.
   */
  bool
  setDataNode(bool flag);

  /**
   * Function to check if the TreeNode has data.
   */
  bool
  isDataNode();

  /**
   * Function to change the revision count of the TreeNode.
   */
  bool
  updateRevisionCount(unsigned long long int revisionCount);

  /**
   * Function to get the revision count of the TreeNode.
   */
  uint64_t
  getRevisionCount();

  /**
   * Function to check if the current node is a leaf node.
   */
  bool
  isLeafNode();

  /**
   * Function to check if the current node is a leaf node.
   */
  bool
  isRootNode();

  /**
   * Function to get number of shared prefix with input name.
   */
  int
  getNumSharedPrefix(TreeNode* node);

  /**
   * Function to remove a child from the current TreeNode. Removing a child
   * changes the treesize of the current node.
   */
  bool
  removeChild(TreeNode* child);

  /**
   * Function to add a child to the current TreeNode. Adding a child
   * changes the treesize of the current node and has an upward spiral
   * affect, i.e., it changes the size of the upper level TreeNodes as well.
   * Complexity O(N).
   */
  bool
  addChild(TreeNode* child);

  /**
   * Function to change the tree size rooted at the current TreeNode.
   * Changing the treesize of the current node and has an upward spiral
   * affect, i.e., it changes the size of the upper level TreeNodes as well.
   * Complexity O(N).
   */
  bool
  setTreeSize(unsigned long long int treeSize);

  /**
   * Function to get the tree size rooted at the current TreeNode.
   */
  uint64_t
  getTreeSize();

  /**
   * Function to print tree nodes data for debugging purposes.
   */
  void
  printTreeNode();

  /**
   * Function to print tree nodes data for debugging purposes.
   */
  void
  printTreeNodeName();

  /**
   * Function to get the parent node.
   */
  TreeNode*
  getParent();
};

} // namespace ndn

#endif // TREE_NODE_HPP

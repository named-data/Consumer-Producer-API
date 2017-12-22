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

#include "infomax-tree-node.hpp"

namespace ndn {

TreeNode::TreeNode(Name& name, TreeNode* parent)
{
  init(name, parent);
}

TreeNode::TreeNode(const TreeNode& other)
  : name(other.name)
  , children(other.children)
  , isMarked(other.isMarked)
  , dataNode(other.dataNode)
  , parent(other.parent)
  , revisionCount(other.revisionCount)
  , treeSize(other.treeSize)
{
}

TreeNode::TreeNode()
{
}

void
TreeNode::init(Name& name, TreeNode* parent)
{
  this->name = name;
  (this->children).clear();

  this->isMarked = false;
  this->dataNode = false;
  this->parent = parent;

  this->revisionCount = 0;
  this->treeSize = 0;
}

Name
TreeNode::getName()
{
  return this->name;
}

std::vector<TreeNode*>
TreeNode::getChildren()
{
  return (this->children);
}

bool
TreeNode::updateRevisionCount(unsigned long long int revisionCount)
{
  this->revisionCount = revisionCount;
  return true;
}

TreeNode*
TreeNode::getParent()
{
  return this->parent;
}

uint64_t
TreeNode::getRevisionCount()
{
  return this->revisionCount;
}

bool
TreeNode::isLeafNode()
{
  if (this->getChildren().empty())
    return true;

  return false;
}

bool
TreeNode::isRootNode()
{
  TreeNode* nullPtr = NULL;
  if (this->getParent() == nullPtr)
    return true;

  return false;
}

bool
TreeNode::setDataNode(bool flag)
{
  this->dataNode = flag;
  return true;
}

bool
TreeNode::isDataNode()
{
  return this->dataNode;
}

bool
TreeNode::markNode(bool status)
{
  this->isMarked = status;
  return true;
}

bool
TreeNode::isNodeMarked()
{
  return this->isMarked;
}

bool
TreeNode::removeChild(TreeNode* child)
{
  if (child == NULL) {
    return false;
  }

  for (unsigned int i = 0; i <= this->children.size(); i++) {
    if (this->children.at(i)->getName().equals(child->getName())) {
      this->children.erase(this->children.begin() + i);
      break;
    }
    else {
      if (i == this->children.size()) {
        return false;
      }
    }
  }

  unsigned long long int newSize = this->parent->getTreeSize() - this->getTreeSize();
  this->parent->setTreeSize(newSize);

  return true;
}

bool
TreeNode::addChild(TreeNode* child)
{
  if (child == NULL) {
    return false;
  }

  (this->children).push_back(child);

  if (child->isDataNode()) {
    unsigned long long int newSize = getTreeSize() + 1;
    this->setTreeSize(newSize);
  }

  return true;
}

bool
TreeNode::setTreeSize(unsigned long long int treeSize)
{
  if (treeSize < (this->treeSize)) {
    return false;
  }
  unsigned long long int difference = treeSize - getTreeSize();
  this->treeSize = treeSize;
  if (parent != NULL) {
    (this->parent)->setTreeSize((this->parent)->getTreeSize() + difference);
  }
  return true;
}

uint64_t
TreeNode::getTreeSize()
{
  return treeSize;
}

int
TreeNode::getNumSharedPrefix(TreeNode* node)
{
  int cnt = 0;
  unsigned int nameSize = std::min(this->getName().size(), node->getName().size());

  for (unsigned int i = 0; i < nameSize; i++) {
    std::string name1 = this->getName().get(i).toUri();
    std::string name2 = node->getName().get(i).toUri();

    if (name1.compare(name2) == 0) {
      cnt++;
    }
    else
      break;
  }
  return cnt;
}

void
TreeNode::printTreeNode()
{
  std::string parentStr = "null";
  if (parent != NULL) {
    parentStr = parent->getName().toUri();
  }

  for (unsigned int i = 0; i < this->children.size(); ++i) {
    TreeNode* n = this->children[i];
    n->printTreeNode();
  }
}

void
TreeNode::printTreeNodeName()
{
  std::string parentStr = "null";
  if (parent != NULL) {
    parentStr = parent->getName().toUri();
  }

  for (unsigned int i = 0; i < this->children.size(); ++i) {
    TreeNode* n = this->children[i];
    n->printTreeNodeName();
  }
}

} // namespace ndn

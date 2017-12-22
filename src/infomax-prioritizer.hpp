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

#ifndef PRIORITIZERS_HPP
#define PRIORITIZERS_HPP

#include "infomax-tree-node.hpp"
#include "producer-context.hpp"
#include <list>

namespace ndn {
class Producer;

class Prioritizer
{
public:
  Prioritizer(Producer* p);

  void
  prioritize();

private:
  void
  simplePrioritizer(TreeNode* root);

  void
  mergePrioritizer(TreeNode* root);

  void
  dummy(TreeNode* root);

  TreeNode*
  getNextPriorityNode(TreeNode* root);

  std::list<TreeNode*>*
  mergeSort(TreeNode* node);

  std::list<TreeNode*>*
  merge(std::vector<std::list<TreeNode*>*>* subTreeMergeList);

  void
  produceInfoMaxList(Name, std::vector<TreeNode*>* prioritizedVector);

  bool
  treeNodePointerComparator(TreeNode* i, TreeNode* j);

  void
  resetNodeStatus(TreeNode* node);

private:
  Producer* m_producer;
  uint64_t m_listVersion;
  Name m_prefix;
  TreeNode m_root;
};

} // namespace ndn
#endif // PRIORITIZERS_HPP

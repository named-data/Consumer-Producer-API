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

#include "infomax-prioritizer.hpp"

namespace ndn {

Prioritizer::Prioritizer(Producer* producer)
{
  m_producer = producer;
  m_listVersion = 0;
}

void
Prioritizer::prioritize()
{
  int type = 0;
  m_producer->getContextOption(PREFIX, m_prefix);
  m_producer->getContextOption(INFOMAX_ROOT, m_root);
  m_producer->getContextOption(INFOMAX_PRIORITY, type);

  m_listVersion++;

  if (type == INFOMAX_SIMPLE_PRIORITY) {
    simplePrioritizer(&m_root);
  }
  else if (type == INFOMAX_MERGE_PRIORITY) {
    mergePrioritizer(&m_root);
  }
  else {
    dummy(&m_root);
  }
}

void
Prioritizer::simplePrioritizer(TreeNode* root)
{
  resetNodeStatus(root);

  unsigned int numOfLeafNodes = root->getTreeSize();
  std::vector<TreeNode*>* prioritizedVector = new std::vector<TreeNode*>();

  while (root->getRevisionCount() < numOfLeafNodes) {
    prioritizedVector->push_back(getNextPriorityNode(root));
  }

  produceInfoMaxList(Name(), prioritizedVector);
}

TreeNode*
Prioritizer::getNextPriorityNode(TreeNode* root)
{
  if (root == 0) {
    return 0;
  }

  root->updateRevisionCount(root->getRevisionCount() + 1);

  if (root->isDataNode() && !(root->isNodeMarked())) {
    root->markNode(true);
    return root;
  }

  std::vector<TreeNode*> children = root->getChildren();

  if (children.size() > 0) {
    uint64_t leastRevisionCountNow = std::numeric_limits<uint64_t>::max();
    ;
    TreeNode* nodeWithLeastCount = NULL;

    for (unsigned int i = 0; i < children.size(); i++) {
      TreeNode* child = children[i];
      if (child->getRevisionCount() < child->getTreeSize() || child->getRevisionCount() == 0) {
        if (nodeWithLeastCount == 0 || (nodeWithLeastCount != 0 && leastRevisionCountNow > child->getRevisionCount())) {
          nodeWithLeastCount = child;
          leastRevisionCountNow = child->getRevisionCount();
        }
      }
    }
    return getNextPriorityNode(nodeWithLeastCount);
  }
  return 0;
}

void
Prioritizer::mergePrioritizer(TreeNode* root)
{
  mergeSort(root);
}

std::list<TreeNode*>*
Prioritizer::mergeSort(TreeNode* node)
{
  std::list<TreeNode*>* mergeList = new std::list<TreeNode*>();
  if (node->isLeafNode()) {
    mergeList->push_back(node);
    return mergeList;
  }

  std::vector<TreeNode*> children = node->getChildren();
  std::vector<std::list<TreeNode*>*>* subTreeMergeList = new std::vector<std::list<TreeNode*>*>();
  for (unsigned int i = 0; i < children.size(); i++) {
    subTreeMergeList->push_back(mergeSort(children[i]));
  }

  mergeList = merge(subTreeMergeList);
  // convert list to std::vector
  std::vector<TreeNode*>* prioritizedVector = new std::vector<TreeNode*>{std::make_move_iterator(std::begin(*mergeList)),
                                                               std::make_move_iterator(std::end(*mergeList))};

  Name subListName = Name();
  if (!node->isRootNode()) {
    subListName.append(node->getName());
  }

  produceInfoMaxList(subListName, prioritizedVector);
  return mergeList;
}

std::list<TreeNode*>*
Prioritizer::merge(std::vector<std::list<TreeNode*>*>* subTreeMergeList)
{
  bool isListAllEmpty = false;
  std::list<TreeNode*>* mergeList = new std::list<TreeNode*>();
  while (!isListAllEmpty) {
    for (unsigned int i = 0; i < subTreeMergeList->size(); i++) {
      if (!subTreeMergeList->at(i)->empty()) {
        isListAllEmpty = false;
        break;
      }
      isListAllEmpty = true;
    }

    for (unsigned int i = 0; i < subTreeMergeList->size(); i++) {
      if (!subTreeMergeList->at(i)->empty()) {
        mergeList->push_back(subTreeMergeList->at(i)->front());
        subTreeMergeList->at(i)->pop_front();
      }
    }
  }

  return mergeList;
}

void
Prioritizer::dummy(TreeNode* root)
{
  std::vector<TreeNode*>* prioritizedVector = new std::vector<TreeNode*>();
  prioritizedVector->push_back(root);

  std::vector<TreeNode*> children = root->getChildren();
  for (unsigned int i = 0; i < children.size(); i++) {
    TreeNode* n = children[i];
    prioritizedVector->push_back(n);
  }

  produceInfoMaxList(root->getName(), prioritizedVector);
}

void
Prioritizer::resetNodeStatus(TreeNode* node)
{
  node->updateRevisionCount(0);
  node->markNode(false);
  std::vector<TreeNode*> children = node->getChildren();

  if (children.size() > 0) {
    for (unsigned int i = 0; i < children.size(); i++) {
      resetNodeStatus(children[i]);
    }
  }
}

void
Prioritizer::produceInfoMaxList(Name prefix, std::vector<TreeNode*>* prioritizedVector)
{
  for (unsigned int i = 0; i < prioritizedVector->size(); i = i + INFOMAX_DEFAULT_LIST_SIZE) {
    uint64_t listNum = i / INFOMAX_DEFAULT_LIST_SIZE + 1;
    Name listName = Name(prefix);
    listName.append(INFOMAX_INTEREST_TAG);
    listName.appendNumber(m_listVersion); // current version all same
    listName.appendNumber(listNum);

    std::string listContent = "";
    for (size_t j = i; j < prioritizedVector->size(); j++) {
      listContent += prioritizedVector->at(j)->getName().getSubName(prefix.size()).toUri();
      listContent += ' ';
    }

    m_producer->produce(listName, (uint8_t*)listContent.c_str(), listContent.size());
  }

  // Produce InfoMax list meta info (version number and the total number of lists)
  Name listMetaInfoName = Name(prefix);
  listMetaInfoName.append(INFOMAX_INTEREST_TAG);
  listMetaInfoName.append(INFOMAX_META_INTEREST_TAG);
  // listMetaInfoName.appendNumber(m_listVersion);

  std::string listMetaInfoContent = "";
  uint64_t totalListNum = prioritizedVector->size() / INFOMAX_DEFAULT_LIST_SIZE + 1;
  listMetaInfoContent = to_string(m_listVersion) + " " + to_string(totalListNum);

  int dataFreshness = 0;
  m_producer->getContextOption(DATA_FRESHNESS, dataFreshness);
  m_producer->setContextOption(DATA_FRESHNESS, 0);
  m_producer->produce(listMetaInfoName, (uint8_t*)listMetaInfoContent.c_str(), listMetaInfoContent.size());
  m_producer->setContextOption(DATA_FRESHNESS, dataFreshness);
}

} // namespace ndn

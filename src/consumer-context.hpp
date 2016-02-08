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

#ifndef CONSUMER_CONTEXT_HPP
#define CONSUMER_CONTEXT_HPP

#include "context.hpp"
#include "context-options.hpp"
#include "context-default-values.hpp"
#include "data-retrieval-protocol.hpp"
#include "simple-data-retrieval.hpp"
#include "unreliable-data-retrieval.hpp"
#include "reliable-data-retrieval.hpp"
#include "infomax-data-retrieval.hpp"

#include <ndn-cxx/util/config-file.hpp>
#include <ndn-cxx/management/nfd-controller.hpp>

namespace ndn {

/**
 * @brief Consumer context is a container of consumer-specific transmission parameters for a 
 * specific name prefix.
 *
 * Consumer context performs fetching of Application Data Units using Interest/Data exchanges. 
 * Consumer context can be tuned using set/getcontextopt primitives.
 */
class Consumer : public Context
{
public:
  /**
   * @brief Initializes consumer context.
   *
   * @param prefix - Name components that define the range of application frames (ADU) 
   *        that can be retrieved from the network.
   * @param protocol - 1) SDR 2) UDR 3) RDR
   */
  explicit Consumer(const Name prefix, int protocol);
  
  /**
   * @brief Stops the ongoing fetching of the Application Data Unit (ADU) and releases all 
   * associated system resources.
   *
   */
  ~Consumer();
  
  /**
   * @brief Performs transmission of Interest packets to fetch specified Application Data Unit (ADU).
   * Consume() blocks until ADU is successfully fetched or an irrecoverable error occurs.
   *
   * @param suffix Name components that identify the boundary of Application Data Unit (ADU)
   */
  int
  consume(Name suffix);
  
  
   /**
   * @brief Performs transmission of Interest packets to fetch specified Application Data Unit (ADU).
   * async_consume() does not block the caller thread. 
   *
   * @param suffix Name components that identify the boundary of Application Data Unit (ADU)
   */
  int
  asyncConsume(Name suffix);
  
  /**
   * @brief Stops the ongoing fetching of the Application Data Unit (ADU).
   *
   */
  void
  stop();

  static void
  consumeAll();

  /* 
   * Context option setters
   * Return OPTION_VALUE_SET if success; otherwise -- OPTION_VALUE_NOT_SET
   */
  int
  setContextOption(int optionName, int optionValue);
  
  int
  setContextOption(int optionName, bool optionValue);
  
  int
  setContextOption(int optionName, size_t optionValue);
  
  int
  setContextOption(int optionName, Name optionValue);
  
  int
  setContextOption(int optionName, ProducerDataCallback optionValue);
  
  int
  setContextOption(int optionName, ConsumerDataVerificationCallback optionValue);
  
  int
  setContextOption(int optionName, ConsumerDataCallback optionValue);
  
  int
  setContextOption(int optionName, ConsumerInterestCallback optionValue);
  
  int
  setContextOption(int optionName, ProducerInterestCallback optionValue);
  
  int
  setContextOption(int optionName, ConsumerContentCallback optionValue);
  
  int
  setContextOption(int optionName, ConsumerNackCallback optionValue);
  
  int
  setContextOption(int optionName, ConsumerManifestCallback optionValue);
  
  int
  setContextOption(int optionName, KeyLocator optionValue);
  
  int
  setContextOption(int optionName, Exclude optionValue);
  
   /* 
   * Context option getters
   * Return OPTION_FOUND if success; otherwise -- OPTION_NOT_FOUND
   */
  int
  getContextOption(int optionName, int& optionValue);
  
  int
  getContextOption(int optionName, size_t& optionValue);

  int
  getContextOption(int optionName, bool& optionValue);
  
  int
  getContextOption(int optionName, Name& optionValue);
  
  int
  getContextOption(int optionName, ProducerDataCallback& optionValue);
  
  int
  getContextOption(int optionName, ConsumerDataVerificationCallback& optionValue);
  
  int
  getContextOption(int optionName, ConsumerDataCallback& optionValue);
  
  int
  getContextOption(int optionName, ConsumerInterestCallback& optionValue);
  
  int
  getContextOption(int optionName, ProducerInterestCallback& optionValue);

  int
  getContextOption(int optionName, ConsumerContentCallback& optionValue);
  
  int
  getContextOption(int optionName, ConsumerNackCallback& optionValue);
  
  int
  getContextOption(int optionName, ConsumerManifestCallback& optionValue);
  
  int
  getContextOption(int optionName, KeyLocator& optionValue);
  
  int
  getContextOption(int optionName, Exclude& optionValue);
  
  int
  getContextOption(int optionName, shared_ptr<Face>& optionValue);
  
  int
  getContextOption(int optionName, TreeNode& optionValue);

private: 

  void
  postponedConsume(Name suffix);

  void
  onStrategyChangeSuccess(const nfd::ControlParameters& commandSuccessResult, const std::string& message);
  
  void
  onStrategyChangeError(uint32_t code, const std::string& error, const std::string& message);

private:
  // context inner state variables
  bool m_isRunning;
  shared_ptr<ndn::Face> m_face;
  shared_ptr<DataRetrievalProtocol> m_dataRetrievalProtocol;
  KeyChain m_keyChain;
  shared_ptr<nfd::Controller> m_controller;
  
  Name m_prefix;
  Name m_suffix;
  Name m_forwardingStrategy;
  
  int m_interestLifetimeMillisec;
  
  int m_minWindowSize;
  int m_maxWindowSize;
  int m_currentWindowSize;
  int m_nMaxRetransmissions;
  int m_nMaxExcludedDigests;
  size_t m_sendBufferSize;
  size_t m_receiveBufferSize;
  
  bool m_isAsync;
  
  /// selectors
  
  int m_minSuffixComponents;
  int m_maxSuffixComponents;
  KeyLocator m_publisherKeyLocator;
  Exclude m_exclude;
  int m_childSelector;
  bool m_mustBeFresh;
  
  /// user-provided callbacks
  
  ConsumerInterestCallback m_onInterestRetransmitted;
  ConsumerInterestCallback m_onInterestToLeaveContext;
  ConsumerInterestCallback m_onInterestExpired;
  ConsumerInterestCallback m_onInterestSatisfied;
  
  ConsumerDataCallback m_onDataEnteredContext;
  ConsumerDataVerificationCallback m_onDataToVerify;
  
  ConsumerDataCallback m_onContentData;
  ConsumerNackCallback m_onNack;
  ConsumerManifestCallback m_onManifest;
  
  ConsumerContentCallback m_onPayloadReassembled;
};
  
} // namespace ndn

#endif // CONSUMER_CONTEXT_HPP

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

#ifndef PRODUCER_CONTEXT_HPP
#define PRODUCER_CONTEXT_HPP

#include "common.hpp"
#include "context-options.hpp"
#include "context-default-values.hpp"
#include "context.hpp"
#include "cs.hpp"
#include "repo-command-parameter.hpp"
#include "infomax-tree-node.hpp"
#include "infomax-prioritizer.hpp"

#include <ndn-cxx/signature.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/management/nfd-controller.hpp>
#include <ndn-cxx/util/scheduler.hpp>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>

#include <queue>
#include <time.h>   // for nanosleep

namespace ndn {
class Prioritizer;
/**
 * @brief Producer context is a container of producer-specific transmission parameters for a 
 * specific name prefix.
 *
 * Producer context performs transformation of Application Data Units into Data packets. 
 * Producer context can be tuned using set/getcontextopt primitives.
 */
class Producer : public Context 
{
public:
  /**
   * @brief Initializes producer context with default parameters.
   * Context's parameters can be changed with setContextOption function.
   *
   * @param prefix Name components that identify the namespace 
   *        where all generated Data packets will be placed.
   */
  explicit Producer(Name prefix);
  
  /**
   * @brief Detaches producer context from the network and releases all associated system resources.
   *
   */
  ~Producer();
  
  /**
   * @brief Attaches producer context to the network by registering its name prefix.
   *
   */
  void
  attach();

  /**
   * @brief Performs segmentation of the supplied memory buffer into Data packets.
   * Produced Data packets are placed in the output buffer to satisfy pending and future Interests.
   * Produce() blocks until all Data segments are succesfully placed in the output buffer.
   *
   * @param suffix Name components that identify the boundary of Application Data Unit (ADU)
   * @param buffer Memory buffer storing Application Data Unit (ADU)
   * @param bufferSize Size of the supplied memory buffer
   *
   * @return The number of produced Data packets or -1 if the supplied ADU requires 
   * more Data segments than it is possible to store in the output buffer.
   */
  void
  produce(Name suffix, const uint8_t* buffer, size_t bufferSize);
  
  void
  produce(Data& packet);

  /**
   * @brief Performs segmentation of the supplied memory buffer into Data packets.
   * Produced Data packets are placed in the output buffer to satisfy pending and future Interests.
   * asyncProduce() does not block and schedules segmentation for future execution.
   *
   * @param suffix Name components that identify the boundary of Application Data Unit (ADU)
   * @param buffer Memory buffer storing Application Data Unit (ADU)
   * @param bufferSize Size of the supplied memory buffer
   *
   */
  void
  asyncProduce(Name suffix, const uint8_t* buffer, size_t bufferSize);
  
  void
  asyncProduce(Data& packet);
  
  /**
   * @brief Satisfies an Interest with Negative Acknowledgement.
   *
   * @param appNack Negative Acknowledgement
   */
  int
  nack(ApplicationNack appNack);
  
  /* 
   * Context option setters
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
  setContextOption(int optionName, ProducerInterestCallback optionValue);

  int
  setContextOption(int optionName, ConsumerDataVerificationCallback optionValue);
  
  int
  setContextOption(int optionName, ConsumerDataCallback optionValue);
  
  int
  setContextOption(int optionName, ConsumerInterestCallback optionValue);
      
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
   */
  
  int
  getContextOption(int optionName, int& optionValue);
  
  int
  getContextOption(int optionName, bool& optionValue);
  
  int
  getContextOption(int optionName, size_t& optionValue);
  
  int
  getContextOption(int optionName, Name& optionValue);
  
  int
  getContextOption(int optionName, ProducerDataCallback& optionValue);
  
  int
  getContextOption(int optionName, ProducerInterestCallback& optionValue);
  
  int
  getContextOption(int optionName, ConsumerDataVerificationCallback& optionValue);
  
  int
  getContextOption(int optionName, ConsumerDataCallback& optionValue);
  
  int
  getContextOption(int optionName, ConsumerInterestCallback& optionValue);
  
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
  // context inner state variables
  ndn::shared_ptr<ndn::Face> m_face;
  boost::asio::io_service m_ioService;
  shared_ptr<nfd::Controller> m_controller;
  Scheduler* m_scheduler;
  
  Name m_prefix;
  Name m_targetRepoPrefix;
  Name m_forwardingStrategy;
  
  int m_dataPacketSize;
  int m_dataFreshness;
  int m_registrationStatus;

  bool m_isMakingManifest;
  
  // repo related stuff
  bool m_isWritingToLocalRepo;
  boost::asio::io_service m_repoIoService;
  boost::asio::ip::tcp::socket m_repoSocket;
  
  // infomax related stuff
  uint64_t m_infomaxTreeVersion;
  uint64_t m_infomaxUpdateInterval;
  bool m_isNewInfomaxData;
  TreeNode m_infomaxRoot;
  shared_ptr<Prioritizer> m_infomaxPrioritizer;
  int m_infomaxType;  // currently there are only 2 types: normal and InfoMax producer
  
  int m_signatureType;
  int m_signatureSize;
  int m_keyLocatorSize;
  KeyLocator m_keyLocator;
  KeyChain m_keyChain;
  
  // buffers
  Cs m_sendBuffer;
  
  //boost::lockfree::queue<shared_ptr<const Interest> >m_receiveBuffer{DEFAULT_PRODUCER_RCV_BUFFER_SIZE};
  std::queue< shared_ptr<const Interest> > m_receiveBuffer;
  boost::mutex m_receiveBufferMutex;
  boost::atomic_size_t m_receiveBufferCapacity;
  boost::atomic_size_t m_receiveBufferSize;
  
  // threads
  boost::thread m_listeningThread;
  boost::thread m_processingThread;
  
  // user-defined callbacks
  ProducerInterestCallback m_onInterestEntersContext;
  ProducerInterestCallback m_onInterestDroppedFromRcvBuffer;
  ProducerInterestCallback m_onInterestPassedRcvBuffer;
  ProducerInterestCallback m_onInterestSatisfiedFromSndBuffer;
  ProducerInterestCallback m_onInterestProcess;
  
  ProducerDataCallback m_onNewSegment;
  ProducerDataCallback m_onDataToSecure;
  ProducerDataCallback m_onDataInSndBuffer;
  ProducerDataCallback m_onDataLeavesContext;
  ProducerDataCallback m_onDataEvictedFromSndBuffer;
  
private:
  void
  listen();
  
  void
  updateInfomaxTree();
  
  void
  onInterest(const Name& name, const Interest& interest);
  
  void
  onRegistrationSucceded (const ndn::Name& prefix);
  
  void
  onRegistrationFailed (const ndn::Name& prefix, const std::string& reason);
  
  void
  processIncomingInterest(/*const Name& name, const Interest& interest*/);

  void
  processInterestFromReceiveBuffer();
  
  void
  passSegmentThroughCallbacks(shared_ptr<Data> segment);
  
  size_t
  estimateManifestSize(shared_ptr<Manifest> manifest);
  
  void
  writeToRepo(Name dataPrefix, uint64_t startSegment, uint64_t endSegment);
  
  void
  onRepoReply(const ndn::Interest& interest, ndn::Data& data);
  
  void
  onRepoTimeout(const ndn::Interest& interest);
  
  void
  onStrategyChangeSuccess(const nfd::ControlParameters& commandSuccessResult, 
                          const std::string& message);
  
  void
  onStrategyChangeError(uint32_t code, const std::string& error, const std::string& message);
};

} // namespace ndn

#endif // PRODUCER_CONTEXT_HPP

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2017 Regents of the University of California.
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

#include "unreliable-data-retrieval.hpp"
#include "consumer-context.hpp"

namespace ndn {

UnreliableDataRetrieval::UnreliableDataRetrieval(Context* context)
  : DataRetrievalProtocol(context)
  , m_isFinalBlockNumberDiscovered(false)
  , m_nTimeouts(0)
  , m_finalBlockNumber(std::numeric_limits<uint64_t>::max())
  , m_segNumber(0)
  , m_currentWindowSize(0)
  , m_interestsInFlight(0)
{
  context->getContextOption(FACE, m_face);
}

void
UnreliableDataRetrieval::start()
{
  m_isRunning = true;
  m_isFinalBlockNumberDiscovered = false;
  m_nTimeouts = 0;
  m_finalBlockNumber = std::numeric_limits<uint64_t>::max();
  m_segNumber = 0;
  m_interestsInFlight = 0;
  m_currentWindowSize = 0;

  // this is to support window size "inheritance" between consume calls
  /*int currentWindowSize = -1;
  m_context->getContextOption(CURRENT_WINDOW_SIZE, currentWindowSize);

  if (currentWindowSize > 0)
  {
    m_currentWindowSize = currentWindowSize;
  }
  else
  {
    int minWindowSize = -1;
    m_context->getContextOption(MIN_WINDOW_SIZE, minWindowSize);

    m_currentWindowSize = minWindowSize;
  }

  // initial burst of Interests
  while (m_interestsInFlight < m_currentWindowSize)
  {
    if (m_isFinalBlockNumberDiscovered)
    {
      if (m_segNumber < m_finalBlockNumber)
      {
        sendInterest();
      }
      else
      {
        break;
      }
    }
    else
    {
      sendInterest();
    }
  }*/

  //send exactly 1 Interest to get the FinalBlockId
  sendInterest();

  bool isAsync = false;
  m_context->getContextOption(ASYNC_MODE, isAsync);

  if (!isAsync) {
    m_face->processEvents();
  }
}

void
UnreliableDataRetrieval::sendInterest()
{
  Name prefix;
  m_context->getContextOption(PREFIX, prefix);

  Name suffix;
  m_context->getContextOption(SUFFIX, suffix);

  if (!suffix.empty()) {
    prefix.append(suffix);
  }

  prefix.appendSegment(m_segNumber);

  Interest interest(prefix);

  int interestLifetime = 0;
  m_context->getContextOption(INTEREST_LIFETIME, interestLifetime);
  interest.setInterestLifetime(time::milliseconds(interestLifetime));

  SelectorHelper::applySelectors(interest, m_context);

  ConsumerInterestCallback onInterestToLeaveContext = EMPTY_CALLBACK;
  m_context->getContextOption(INTEREST_LEAVE_CNTX, onInterestToLeaveContext);
  if (onInterestToLeaveContext != EMPTY_CALLBACK) {
    onInterestToLeaveContext(*dynamic_cast<Consumer*>(m_context), interest);
  }

  m_interestsInFlight++;
  m_expressedInterests[m_segNumber] = m_face->expressInterest(interest,
                                                              bind(&UnreliableDataRetrieval::onData, this, _1, _2),
                                                              bind(&UnreliableDataRetrieval::onNack, this, _1, _2),
                                                              bind(&UnreliableDataRetrieval::onTimeout, this, _1));
  m_segNumber++;
}


void
UnreliableDataRetrieval::stop()
{
  m_isRunning = false;
  removeAllPendingInterests();
}

void
UnreliableDataRetrieval::onData(const Interest& interest, const Data& data)
{
  if (m_isRunning == false)
    return;

  m_interestsInFlight--;

  ConsumerDataCallback onDataEnteredContext = EMPTY_CALLBACK;
  m_context->getContextOption(DATA_ENTER_CNTX, onDataEnteredContext);
  if (onDataEnteredContext != EMPTY_CALLBACK) {
    onDataEnteredContext(*dynamic_cast<Consumer*>(m_context), data);
  }

  ConsumerInterestCallback onInterestSatisfied = EMPTY_CALLBACK;
  m_context->getContextOption(INTEREST_SATISFIED, onInterestSatisfied);
  if (onInterestSatisfied != EMPTY_CALLBACK) {
    onInterestSatisfied(*dynamic_cast<Consumer*>(m_context), const_cast<Interest&>(interest));
  }

  ConsumerDataVerificationCallback onDataToVerify = EMPTY_CALLBACK;
  m_context->getContextOption(DATA_TO_VERIFY, onDataToVerify);

  bool isDataSecure = false;
  if (onDataToVerify == EMPTY_CALLBACK) {
    isDataSecure = true;
  }
  else {
    if (onDataToVerify(*dynamic_cast<Consumer*>(m_context), data) == true) // runs verification routine
    {
      isDataSecure = true;
    }
  }

  if (isDataSecure) {
    checkFastRetransmissionConditions(interest);

    if (data.getContentType() == CONTENT_DATA_TYPE) {
      int maxWindowSize = -1;
      m_context->getContextOption(MAX_WINDOW_SIZE, maxWindowSize);
      if (m_currentWindowSize < maxWindowSize) {
        m_currentWindowSize++;
      }

      if (!data.getFinalBlockId().empty()) {
        m_isFinalBlockNumberDiscovered = true;
        m_finalBlockNumber = data.getFinalBlockId().toSegment();
      }

      const Block content = data.getContent();

      ConsumerContentCallback onPayload = EMPTY_CALLBACK;
      m_context->getContextOption(CONTENT_RETRIEVED, onPayload);
      if (onPayload != EMPTY_CALLBACK) {
        onPayload(*dynamic_cast<Consumer*>(m_context), content.value(), content.value_size());
      }
    }
    else if (data.getContentType() == NACK_DATA_TYPE) {
      int minWindowSize = -1;
      m_context->getContextOption(MIN_WINDOW_SIZE, minWindowSize);
      if (m_currentWindowSize > minWindowSize) {
        m_currentWindowSize = m_currentWindowSize / 2; // cut in half
        if (m_currentWindowSize == 0)
          m_currentWindowSize++;
      }

      shared_ptr<ApplicationNack> nack = make_shared<ApplicationNack>(data);

      ConsumerNackCallback onNack = EMPTY_CALLBACK;
      m_context->getContextOption(NACK_ENTER_CNTX, onNack);
      if (onNack != EMPTY_CALLBACK) {
        onNack(*dynamic_cast<Consumer*>(m_context), *nack);
      }
    }
  }

  if (!m_isRunning || ((m_isFinalBlockNumberDiscovered) && (data.getName().get(-1).toSegment() >= m_finalBlockNumber))) {
    removeAllPendingInterests();
    m_isRunning = false;

    //reduce window size to prevent its speculative growth in case when consume() is called in loop
    int currentWindowSize = -1;
    m_context->getContextOption(CURRENT_WINDOW_SIZE, currentWindowSize);
    if (currentWindowSize > m_finalBlockNumber) {
      m_context->setContextOption(CURRENT_WINDOW_SIZE, (int)(m_finalBlockNumber));
    }
  }

  // some flow control
  while (m_interestsInFlight < m_currentWindowSize) {
    if (m_isFinalBlockNumberDiscovered) {
      if (m_segNumber <= m_finalBlockNumber) {
        sendInterest();
      }
      else {
        break;
      }
    }
    else {
      sendInterest();
    }
  }
}

void
UnreliableDataRetrieval::onNack(const Interest& interest, const lp::Nack& data)
{
  // TODO something more meaningful, may fail
  return onTimeout(interest);
}

void
UnreliableDataRetrieval::onTimeout(const Interest& interest)
{
  if (m_isRunning == false)
    return;

  m_interestsInFlight--;

  int minWindowSize = -1;
  m_context->getContextOption(MIN_WINDOW_SIZE, minWindowSize);
  if (m_currentWindowSize > minWindowSize) {
    m_currentWindowSize = m_currentWindowSize / 2; // cut in half
    if (m_currentWindowSize == 0)
      m_currentWindowSize++;
  }

  ConsumerInterestCallback onInterestExpired = EMPTY_CALLBACK;
  m_context->getContextOption(INTEREST_EXPIRED, onInterestExpired);
  if (onInterestExpired != EMPTY_CALLBACK) {
    onInterestExpired(*dynamic_cast<Consumer*>(m_context), const_cast<Interest&>(interest));
  }

  // this code handles the situation when an application frame is small (1 or several packets)
  // and packets are lost. Without this code, the protocol continues to send Interests for
  // non-existing packets, because it was never able to discover the correct FinalBlockID.

  if (!m_isFinalBlockNumberDiscovered) {
    m_nTimeouts++;
    if (m_nTimeouts > 2) {
      m_isRunning = false;
      return;
    }
  }

  // some flow control
  while (m_interestsInFlight < m_currentWindowSize) {
    //std::cout << "inFlight: " << m_interestsInFlight << " windSize " << m_currentWindowSize << std::endl;
    if (m_isFinalBlockNumberDiscovered) {
      if (m_segNumber <= m_finalBlockNumber) {
        sendInterest();
      }
      else {
        break;
      }
    }
    else {
      sendInterest();
    }
  }
}

void
UnreliableDataRetrieval::checkFastRetransmissionConditions(const Interest& interest)
{
  uint64_t segNumber = interest.getName().get(-1).toSegment();
  m_receivedSegments[segNumber] = true;
  m_fastRetxSegments.erase(segNumber);

  uint64_t possiblyLostSegment = 0;
  uint64_t highestReceivedSegment = m_receivedSegments.rbegin()->first;

  for (uint64_t i = 0; i <= highestReceivedSegment; i++) {
    if (m_receivedSegments.find(i) == m_receivedSegments.end()) // segment is not received yet
    {
      // segment has not been fast retransmitted yet
      if (m_fastRetxSegments.find(i) == m_fastRetxSegments.end()) {
        possiblyLostSegment = i;
        uint8_t nOutOfOrderSegments = 0;
        for (uint64_t j = i; j <= highestReceivedSegment; j++) {
          if (m_receivedSegments.find(j) != m_receivedSegments.end()) {
            nOutOfOrderSegments++;
            if (nOutOfOrderSegments == DEFAULT_FAST_RETX_CONDITION) {
              m_fastRetxSegments[possiblyLostSegment] = true;
              fastRetransmit(interest, possiblyLostSegment);
            }
          }
        }
      }
    }
  }
}

void
UnreliableDataRetrieval::fastRetransmit(const Interest& interest, uint64_t segNumber)
{
  Name name = interest.getName().getPrefix(-1);
  name.appendSegment(segNumber);

  Interest retxInterest(name);
  SelectorHelper::applySelectors(retxInterest, m_context);

  ConsumerInterestCallback onInterestRetransmitted = EMPTY_CALLBACK;
  m_context->getContextOption(INTEREST_RETRANSMIT, onInterestRetransmitted);

  if (onInterestRetransmitted != EMPTY_CALLBACK) {
    onInterestRetransmitted(*dynamic_cast<Consumer*>(m_context), retxInterest);
  }

  ConsumerInterestCallback onInterestToLeaveContext = EMPTY_CALLBACK;
  m_context->getContextOption(INTEREST_LEAVE_CNTX, onInterestToLeaveContext);
  if (onInterestToLeaveContext != EMPTY_CALLBACK) {
    onInterestToLeaveContext(*dynamic_cast<Consumer*>(m_context), retxInterest);
  }

  //retransmit
  m_interestsInFlight++;
  m_expressedInterests[m_segNumber] = m_face->expressInterest(retxInterest,
                                                              bind(&UnreliableDataRetrieval::onData, this, _1, _2),
                                                              bind(&UnreliableDataRetrieval::onNack, this, _1, _2),
                                                              bind(&UnreliableDataRetrieval::onTimeout, this, _1));
}

void
UnreliableDataRetrieval::removeAllPendingInterests()
{
  bool isAsync = false;
  m_context->getContextOption(ASYNC_MODE, isAsync);

  if (!isAsync) {
    //won't work ---> m_face->getIoService().stop();
    m_face->removeAllPendingInterests(); // faster, but destroys everything
  }
  else // slower, but destroys only necessary Interests
  {
    for (std::unordered_map<uint64_t, const PendingInterestId*>::iterator it = m_expressedInterests.begin(); it != m_expressedInterests.end();
         ++it) {
      m_face->removePendingInterest(it->second);
    }
  }

  m_expressedInterests.clear();
}

} //namespace ndn

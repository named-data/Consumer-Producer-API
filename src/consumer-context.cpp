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

#include "consumer-context.hpp"

namespace ndn {

Consumer::Consumer(Name prefix, int protocol)
  : m_isRunning(false)
  , m_prefix(prefix)
  , m_interestLifetimeMillisec(DEFAULT_INTEREST_LIFETIME)
  , m_minWindowSize(DEFAULT_MIN_WINDOW_SIZE)
  , m_maxWindowSize(DEFAULT_MAX_WINDOW_SIZE)
  , m_currentWindowSize(-1)
  , m_nMaxRetransmissions(CONSUMER_MAX_RETRANSMISSIONS)
  , m_nMaxExcludedDigests(DEFAULT_MAX_EXCLUDED_DIGESTS)
  , m_isAsync(false)
  , m_minSuffixComponents(DEFAULT_MIN_SUFFIX_COMP)
  , m_maxSuffixComponents(DEFAULT_MAX_SUFFIX_COMP)
  , m_childSelector(0)
  , m_mustBeFresh(false)
  , m_onInterestToLeaveContext(EMPTY_CALLBACK)
  , m_onInterestExpired(EMPTY_CALLBACK)
  , m_onInterestSatisfied(EMPTY_CALLBACK)
  , m_onDataEnteredContext(EMPTY_CALLBACK)
  , m_onDataToVerify(EMPTY_CALLBACK)
  , m_onContentData(EMPTY_CALLBACK)
  , m_onNack(EMPTY_CALLBACK)
  , m_onManifest(EMPTY_CALLBACK)
  , m_onPayloadReassembled(EMPTY_CALLBACK)
{    
  m_face = ndn::make_shared<Face>();
  //m_ioService = ndn::make_shared<boost::asio::io_service>();
  m_controller = ndn::make_shared<nfd::Controller>(*m_face, m_keyChain);
  
  if (protocol == UDR)
  {
    m_dataRetrievalProtocol = make_shared<UnreliableDataRetrieval>(this);
  }
  else if (protocol == RDR)
  {
    m_dataRetrievalProtocol = make_shared<ReliableDataRetrieval>(this);
  }
  else if (protocol == IDR)
  {
    m_dataRetrievalProtocol = make_shared<InfoMaxDataRetrieval>(this);
  }
  else
  {
    m_dataRetrievalProtocol = make_shared<SimpleDataRetrieval>(this);
  }
}

Consumer::~Consumer()
{
  stop();
  m_dataRetrievalProtocol.reset();  // reset the pointer counter
  m_face.reset(); // reset the pointer counter
}

int
Consumer::consume(Name suffix)
{
  if (m_isRunning/*m_dataRetrievalProtocol->isRunning()*/)
  {
    // put in the schedule
    m_face->getIoService().post(bind(&Consumer::postponedConsume, this, suffix));
    return CONSUMER_BUSY;
  }
  
  // if previously used in non-blocking mode
  if (m_isAsync) 
  {
    m_face = ndn::make_shared<Face>();
    m_controller = ndn::make_shared<nfd::Controller>(*m_face, m_keyChain);
    m_dataRetrievalProtocol->updateFace();
  }
  
  m_suffix = suffix;
  m_isAsync = false;
  m_dataRetrievalProtocol->start();
  m_isRunning = false;
  return CONSUMER_READY;
}

void
Consumer::postponedConsume(Name suffix)
{
  // if previously used in non-blocking mode
  if (m_isAsync) 
  {
    m_face = ndn::make_shared<Face>();
    m_controller = ndn::make_shared<nfd::Controller>(*m_face, m_keyChain);
    m_dataRetrievalProtocol->updateFace();
  }
  
  m_suffix = suffix;
  m_isAsync = false;
  m_dataRetrievalProtocol->start();
}

int
Consumer::asyncConsume(Name suffix)
{
  if (m_dataRetrievalProtocol->isRunning())
  {
    return CONSUMER_BUSY;
  }

  if (!m_isAsync) // if previously used in blocking mode
  {
    m_face = FaceHelper::getFace();
    m_controller = ndn::make_shared<nfd::Controller>(*m_face, m_keyChain);
    m_dataRetrievalProtocol->updateFace();
  }

  m_suffix = suffix;
  m_isAsync = true;
  m_dataRetrievalProtocol->start();
  return CONSUMER_READY;
}

void
Consumer::stop()
{
  if (m_dataRetrievalProtocol->isRunning())
  {
    m_dataRetrievalProtocol->stop();
    m_face->getIoService().stop();
    m_face->getIoService().reset();
  }
  
  m_isRunning = false;
}

int
Consumer::setContextOption(int optionName, int optionValue)
{
  switch (optionName)
  {
    case MIN_WINDOW_SIZE:
      m_minWindowSize = optionValue;
      return OPTION_VALUE_SET;
      
    case MAX_WINDOW_SIZE:
      m_maxWindowSize = optionValue;
      return OPTION_VALUE_SET;
    
    case MAX_EXCLUDED_DIGESTS:
      m_nMaxExcludedDigests = optionValue;
      return OPTION_VALUE_SET;
    
    case CURRENT_WINDOW_SIZE:
      m_currentWindowSize = optionValue;
      return OPTION_VALUE_SET;
    
    case RCV_BUF_SIZE:
      m_receiveBufferSize = optionValue;
      return OPTION_VALUE_SET;

    case SND_BUF_SIZE:
      m_sendBufferSize = optionValue;
      return OPTION_VALUE_SET;
  
    case INTEREST_RETX:
      if (optionValue < CONSUMER_MAX_RETRANSMISSIONS)
      {
        m_nMaxRetransmissions = optionValue;
        return OPTION_VALUE_SET;
      }
      else
      {
        return OPTION_VALUE_NOT_SET;
      }
      
    case INTEREST_LIFETIME:
      m_interestLifetimeMillisec = optionValue;
      return OPTION_VALUE_SET;
      
    case MIN_SUFFIX_COMP_S:
      if (optionValue >= 0)
      {
        m_minSuffixComponents = optionValue;
      }
      else
      {
        m_minSuffixComponents = DEFAULT_MIN_SUFFIX_COMP;
        return OPTION_VALUE_NOT_SET;
      }
      
    case MAX_SUFFIX_COMP_S:
      if (optionValue >= 0)
      {
        m_maxSuffixComponents = optionValue;
      }
      else
      {
        m_maxSuffixComponents = DEFAULT_MAX_SUFFIX_COMP;
        return OPTION_VALUE_NOT_SET;
      }
      
    case RIGHTMOST_CHILD_S:
      if (optionValue == 1)
      {
        m_childSelector = 1;
      }
      else
      {
        m_childSelector = 0;
      }
      return OPTION_VALUE_SET;
      
    case LEFTMOST_CHILD_S:
      if (optionValue == 1)
      {
        m_childSelector = 0;
      }
      else
      {
        m_childSelector = 1;
      }
      return OPTION_VALUE_SET;
      
    case INTEREST_RETRANSMIT:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onInterestRetransmitted = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }
  
    case INTEREST_EXPIRED:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onInterestExpired = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }
  
    case INTEREST_SATISFIED:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onInterestSatisfied = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }
  
    case INTEREST_LEAVE_CNTX:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onInterestToLeaveContext = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }

    case DATA_ENTER_CNTX:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onDataEnteredContext = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }
  
    case DATA_TO_VERIFY:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onDataToVerify = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }
  
    case CONTENT_RETRIEVED:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onPayloadReassembled = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }
  
    default:
      return OPTION_VALUE_NOT_SET;
  }
}

int
Consumer::setContextOption(int optionName, size_t optionValue)
{
  switch (optionName)
  {
    case RCV_BUF_SIZE:
      m_receiveBufferSize = optionValue;
      return OPTION_VALUE_SET;

    case SND_BUF_SIZE:
      m_sendBufferSize = optionValue;
      return OPTION_VALUE_SET;
  
    default:
      return OPTION_VALUE_NOT_SET;
  }
}
  
int
Consumer::setContextOption(int optionName, bool optionValue)
{
  switch (optionName)
  {
    case MUST_BE_FRESH_S:
      m_mustBeFresh = optionValue;
      return OPTION_VALUE_SET;
    
    case RIGHTMOST_CHILD_S:
      if (optionValue == true)
      {
        m_childSelector = 1;
      }
      else
      {
        m_childSelector = 0;
      }
      return OPTION_VALUE_SET;
      
    case LEFTMOST_CHILD_S:
      if (optionValue == true)
      {
        m_childSelector = 0;
      }
      else
      {
        m_childSelector = 1;
      }
      return OPTION_VALUE_SET;   
    
    case RUNNING:
      m_isRunning = optionValue;
      return OPTION_VALUE_SET;
                      
    default:
      return OPTION_VALUE_NOT_SET;
  }
}
  
int
Consumer::setContextOption(int optionName, Name optionValue)
{
  switch (optionName)
  {
    case PREFIX:
      m_prefix = optionValue;;
      return OPTION_VALUE_SET;

    case SUFFIX:
      m_suffix = optionValue;
      return OPTION_VALUE_SET;
  
    case FORWARDING_STRATEGY:
      m_forwardingStrategy = optionValue;
      if (m_forwardingStrategy.empty())
      {
        nfd::ControlParameters parameters;
        parameters.setName(m_prefix);

        m_controller->start<nfd::StrategyChoiceUnsetCommand>(parameters,
                                                 bind(&Consumer::onStrategyChangeSuccess, this, _1,
                                                      "Successfully unset strategy choice"),
                                                 bind(&Consumer::onStrategyChangeError, this, _1, _2,
                                                      "Failed to unset strategy choice"));
      }
      else
      {
        nfd::ControlParameters parameters;
        parameters
          .setName(m_prefix)
          .setStrategy(m_forwardingStrategy);

        m_controller->start<nfd::StrategyChoiceSetCommand>(parameters,
                                               bind(&Consumer::onStrategyChangeSuccess, this, _1,
                                                    "Successfully set strategy choice"),
                                               bind(&Consumer::onStrategyChangeError, this, _1, _2,
                                                    "Failed to set strategy choice"));

      }
      return OPTION_VALUE_SET;
  
    default:
      return OPTION_VALUE_NOT_SET;
  }
}

int
Consumer::setContextOption(int optionName, ConsumerDataCallback optionValue)
{
  switch (optionName)
  {
    case DATA_ENTER_CNTX:
      m_onDataEnteredContext = optionValue;;
      return OPTION_VALUE_SET;
  
    default:
      return OPTION_VALUE_NOT_SET;
  }
}

int
Consumer::setContextOption(int optionName, ProducerDataCallback optionValue)
{
  return OPTION_VALUE_NOT_SET;
}


int
Consumer::setContextOption(int optionName, ConsumerDataVerificationCallback optionValue)
{
  switch (optionName)
  {
    case DATA_TO_VERIFY:
      m_onDataToVerify = optionValue;
      return OPTION_VALUE_SET;
    
    default:
      return OPTION_VALUE_NOT_SET;
  }
}

int
Consumer::setContextOption(int optionName, ConsumerInterestCallback optionValue)
{
  switch (optionName)
  {
    case INTEREST_RETRANSMIT:
      m_onInterestRetransmitted = optionValue;
      return OPTION_VALUE_SET;
      
    case INTEREST_LEAVE_CNTX:
      m_onInterestToLeaveContext = optionValue;
      return OPTION_VALUE_SET;
  
    case INTEREST_EXPIRED:
      m_onInterestExpired = optionValue;
      return OPTION_VALUE_SET;
      
    case INTEREST_SATISFIED:
      m_onInterestSatisfied = optionValue;
      return OPTION_VALUE_SET;
  
    default:
      return OPTION_VALUE_NOT_SET;
  }
}

int
Consumer::setContextOption(int optionName, ProducerInterestCallback optionValue)
{
  return OPTION_VALUE_NOT_SET;
}

int
Consumer::setContextOption(int optionName, ConsumerContentCallback optionValue)
{
  switch (optionName)
  {
    case CONTENT_RETRIEVED:
      m_onPayloadReassembled = optionValue;
      return OPTION_VALUE_SET;
    
    default: return OPTION_VALUE_NOT_SET;
  }
}

int
Consumer::setContextOption(int optionName, ConsumerNackCallback optionValue)
{
  switch (optionName)
  {
    case NACK_ENTER_CNTX:
      m_onNack = optionValue;
      return OPTION_VALUE_SET;
    
    default: return OPTION_VALUE_NOT_SET;
  }
}

int
Consumer::setContextOption(int optionName, ConsumerManifestCallback optionValue)
{
  switch (optionName)
  {
    case MANIFEST_ENTER_CNTX:
      m_onManifest = optionValue;
      return OPTION_VALUE_SET;
    
    default: return OPTION_VALUE_NOT_SET;
  }
}


int
Consumer::setContextOption(int optionName, KeyLocator optionValue)
{
  return OPTION_VALUE_NOT_SET;
}
  
int
Consumer::setContextOption(int optionName, Exclude optionValue)
{
  switch (optionName)
  {
    case EXCLUDE_S:
      m_exclude = optionValue;
      return OPTION_VALUE_SET;
    
    default: return OPTION_VALUE_NOT_SET;
  }
}

int
Consumer::getContextOption(int optionName, int& optionValue)
{
  switch (optionName)
  {
    case MIN_WINDOW_SIZE:
      optionValue = m_minWindowSize;
      return OPTION_FOUND;
      
    case MAX_WINDOW_SIZE:
      optionValue = m_maxWindowSize;
      return OPTION_FOUND;
      
    case MAX_EXCLUDED_DIGESTS:
      optionValue = m_nMaxExcludedDigests;
      return OPTION_FOUND;
    
    case CURRENT_WINDOW_SIZE:
      optionValue = m_currentWindowSize;
      return OPTION_FOUND;
  
    case RCV_BUF_SIZE:
      optionValue = m_receiveBufferSize;
      return OPTION_FOUND;

    case SND_BUF_SIZE:
      optionValue = m_sendBufferSize;
      return OPTION_FOUND;
  
    case INTEREST_RETX:
      optionValue = m_nMaxRetransmissions;
      return OPTION_FOUND;
      
    case INTEREST_LIFETIME:
      optionValue = m_interestLifetimeMillisec;
      return OPTION_FOUND;
      
    case MIN_SUFFIX_COMP_S:
      optionValue = m_minSuffixComponents;
      return OPTION_FOUND;
      
    case MAX_SUFFIX_COMP_S:
      optionValue = m_maxSuffixComponents;
      return OPTION_FOUND;
      
    case RIGHTMOST_CHILD_S:
      optionValue = m_childSelector;
      return OPTION_FOUND;
    
    default:
      return OPTION_NOT_FOUND;
  }
}

int
Consumer::getContextOption(int optionName, size_t& optionValue)
{
  switch (optionName)
  {
    case RCV_BUF_SIZE:
      optionValue = m_receiveBufferSize;
      return OPTION_FOUND;

    case SND_BUF_SIZE:
      optionValue = m_sendBufferSize;
      return OPTION_FOUND;
  
    default:
      return OPTION_NOT_FOUND;
  }
}
  
int
Consumer::getContextOption(int optionName, bool& optionValue)
{
  switch (optionName)
  {    
    case MUST_BE_FRESH_S:
      optionValue = m_mustBeFresh;
      return OPTION_FOUND;

    case ASYNC_MODE:
      optionValue = m_isAsync;
      return OPTION_FOUND;
    
    case RUNNING:
      optionValue = m_isRunning;
      return OPTION_FOUND;
    
    default:
      return OPTION_NOT_FOUND;
  }
}
  
int
Consumer::getContextOption(int optionName, Name& optionValue)
{
  switch (optionName)
  {
    case PREFIX:
      optionValue = m_prefix;
      return OPTION_FOUND;
      
    case SUFFIX:
      optionValue = m_suffix;
      return OPTION_FOUND;
    
    case FORWARDING_STRATEGY:
      optionValue = m_forwardingStrategy;
      return OPTION_FOUND;
    
    default:
      return OPTION_NOT_FOUND;
  }
}

int
Consumer::getContextOption(int optionName, ConsumerDataCallback& optionValue)
{
  switch (optionName)
  {
    case DATA_ENTER_CNTX:
      optionValue = m_onDataEnteredContext;
      return OPTION_FOUND;

    default:
      return OPTION_NOT_FOUND;
  }
}

int
Consumer::getContextOption(int optionName, ProducerDataCallback& optionValue)
{
  return OPTION_NOT_FOUND;
}

  
int
Consumer::getContextOption(int optionName, ConsumerDataVerificationCallback& optionValue)
{
  switch (optionName)
  {
    case DATA_TO_VERIFY:
      optionValue = m_onDataToVerify;
      return OPTION_FOUND;

    default:
      return OPTION_NOT_FOUND;
  }
}

int
Consumer::getContextOption(int optionName, ConsumerInterestCallback& optionValue)
{
  switch (optionName)
  {
    case INTEREST_RETRANSMIT:
      optionValue = m_onInterestRetransmitted;
      return OPTION_FOUND;
      
    case INTEREST_LEAVE_CNTX:
      optionValue = m_onInterestToLeaveContext;
      return OPTION_FOUND;
  
     case INTEREST_EXPIRED:
      optionValue = m_onInterestExpired;
      return OPTION_FOUND;
      
    case INTEREST_SATISFIED:
      optionValue = m_onInterestSatisfied;
      return OPTION_FOUND;
  
    default:
      return OPTION_NOT_FOUND;
  }
}

int
Consumer::getContextOption(int optionName, ProducerInterestCallback& optionValue)
{
  return OPTION_NOT_FOUND;
}

int
Consumer::getContextOption(int optionName, ConsumerContentCallback& optionValue)
{
  switch (optionName)
  {
    case CONTENT_RETRIEVED:
      optionValue = m_onPayloadReassembled;
      return OPTION_FOUND;
    
    default: return OPTION_NOT_FOUND;
  }
}

int
Consumer::getContextOption(int optionName, ConsumerNackCallback& optionValue)
{
  switch (optionName)
  {
    case NACK_ENTER_CNTX:
      optionValue = m_onNack;
      return OPTION_FOUND;
    
    default: return OPTION_NOT_FOUND;
  }
}

int
Consumer::getContextOption(int optionName, ConsumerManifestCallback& optionValue)
{
  switch (optionName)
  {
    case MANIFEST_ENTER_CNTX:
      optionValue = m_onManifest;
      return OPTION_FOUND;
    
    default: return OPTION_NOT_FOUND;
  }
}

int
Consumer::getContextOption(int optionName, KeyLocator& optionValue)
{
  switch (optionName)
  {
    case KEYLOCATOR_S:
      optionValue = m_publisherKeyLocator;
      return OPTION_FOUND;
    
    default: return OPTION_NOT_FOUND;
  }
}
  
int
Consumer::getContextOption(int optionName, Exclude& optionValue)
{
  switch (optionName)
  {
    case EXCLUDE_S:
      optionValue = m_exclude;
      return OPTION_FOUND;
    
    default: return OPTION_NOT_FOUND;
  }
}

int
Consumer::getContextOption(int optionName, shared_ptr<Face>& optionValue)
{
  switch (optionName)
  {
    case FACE:
      optionValue = m_face;
      return OPTION_FOUND;
    
    default: return OPTION_NOT_FOUND;
  }
}

int
Consumer::getContextOption(int optionName, TreeNode& optionValue)
{
  return OPTION_NOT_FOUND;
}

void
Consumer::onStrategyChangeSuccess(const nfd::ControlParameters& commandSuccessResult, 
                                  const std::string& message)
{
  //std::cout << message << ": " << commandSuccessResult << std::endl;
}

void
Consumer::onStrategyChangeError(uint32_t code, const std::string& error, const std::string& message)
{
  /*std::ostringstream os;
  os << message << ": " << error << " (code: " << code << ")";
  throw Error(os.str());*/
}

void
Consumer::consumeAll()
{
  shared_ptr<Face> face = FaceHelper::getFace();
  face->processEvents();
}

} //namespace ndn

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

#include "producer-context.hpp"

namespace ndn {

Producer::Producer(Name prefix)
: m_prefix(prefix)
, m_dataPacketSize(DEFAULT_DATA_PACKET_SIZE)
, m_dataFreshness(DEFAULT_DATA_FRESHNESS)
, m_registrationStatus(REGISTRATION_NOT_ATTEMPTED)
, m_isMakingManifest(false)
, m_isWritingToLocalRepo(false)
, m_repoSocket(m_repoIoService)
, m_infomaxType (INFOMAX_NONE) // infomax disabled by default
, m_infomaxTreeVersion (0)
, m_infomaxUpdateInterval (INFOMAX_DEFAULT_UPDATE_INTERVAL)
, m_isNewInfomaxData (false)
, m_infomaxRoot (TreeNode (prefix, 0))
, m_signatureType(SHA_256)
, m_keyLocatorSize(DEFAULT_KEY_LOCATOR_SIZE)
, m_sendBuffer(DEFAULT_PRODUCER_SND_BUFFER_SIZE)
, m_receiveBufferCapacity(DEFAULT_PRODUCER_RCV_BUFFER_SIZE)
, m_receiveBufferSize(0)
, m_onInterestEntersContext(EMPTY_CALLBACK)
, m_onInterestDroppedFromRcvBuffer(EMPTY_CALLBACK)
, m_onInterestPassedRcvBuffer(EMPTY_CALLBACK)
, m_onInterestSatisfiedFromSndBuffer(EMPTY_CALLBACK)
, m_onInterestProcess(EMPTY_CALLBACK)
, m_onNewSegment(EMPTY_CALLBACK)
, m_onDataToSecure(EMPTY_CALLBACK)
, m_onDataInSndBuffer(EMPTY_CALLBACK)
, m_onDataLeavesContext(EMPTY_CALLBACK)
, m_onDataEvictedFromSndBuffer(EMPTY_CALLBACK)
{
  m_face = ndn::make_shared<ndn::Face>();
  m_controller = ndn::make_shared<nfd::Controller>(*m_face, m_keyChain);
  m_scheduler = new Scheduler(m_face->getIoService());
}

Producer::~Producer()
{
  m_repoSocket.close();
  m_listeningThread.interrupt();
  delete m_scheduler;
  m_controller.reset();
  m_face.reset();
}

void
Producer::attach()
{
  m_listeningThread = boost::thread(bind(&Producer::listen, this));
  m_processingThread = boost::thread(bind(&Producer::processIncomingInterest, this));
}

void
Producer::listen()
{
  m_registrationStatus = REGISTRATION_IN_PROGRESS;
  m_face->setInterestFilter(m_prefix,
                            bind(&Producer::onInterest, this, _1, _2),
                            bind(&Producer::onRegistrationSucceded, this, _1),
                            bind(&Producer::onRegistrationFailed, this, _1, _2));
  
  m_face->processEvents();
}

void
Producer::updateInfomaxTree()
{
  if (m_infomaxType == INFOMAX_SIMPLE_PRIORITY || m_infomaxType == INFOMAX_MERGE_PRIORITY)
  {
    m_scheduler->scheduleEvent( time::milliseconds(m_infomaxUpdateInterval), 
                                bind(&Producer::updateInfomaxTree, this));
  }
  
  if (!m_isNewInfomaxData || m_infomaxType == INFOMAX_NONE)
  {
    return;
  }

  m_infomaxPrioritizer->prioritize();
  m_isNewInfomaxData = false;
}

void
Producer::onRegistrationSucceded (const ndn::Name& prefix)
{
  m_registrationStatus = REGISTRATION_SUCCESS;
}

void
Producer::onRegistrationFailed (const ndn::Name& prefix, const std::string& reason)
{
  m_registrationStatus = REGISTRATION_FAILURE;
  m_face->shutdown();
}

void
Producer::passSegmentThroughCallbacks(shared_ptr<Data> segment)
{
  if (segment)
  {
    if (m_onNewSegment != EMPTY_CALLBACK)
    {
      m_onNewSegment(*this, *segment);
    }
        
    if (m_onDataToSecure != EMPTY_CALLBACK)
    {
      if (!m_isMakingManifest)
      {
        m_onDataToSecure(*this, *segment);
      }
      else
      {
        if (segment->getContentType() == tlv::ContentType_Manifest)
        {
          m_onDataToSecure(*this, *segment);
        }
        else
        {
          // data's KeyLocator will point to the corresponding manifest
          DigestSha256 sig;
          const SignatureInfo info(tlv::DigestSha256, m_keyLocator);
          sig.setInfo(info);
          segment->setSignature(sig);

          Block sigValue(tlv::SignatureValue,
                 crypto::sha256(segment->wireEncode().value(),
                                segment->wireEncode().value_size() -
                                segment->getSignature().getValue().size()));
          segment->setSignatureValue(sigValue);
        }
      }
    }
    else // this is for developers who don't care about security
    {
      m_keyChain.signWithSha256(*segment);
    }

    if (m_onDataInSndBuffer != EMPTY_CALLBACK)
    {
      m_onDataInSndBuffer(*this, *segment);
    }
       
    m_sendBuffer.insert(*segment);
    
    if (m_onDataLeavesContext != EMPTY_CALLBACK)
    {
      m_onDataLeavesContext(*this, *segment);
    }
    
    m_face->put(*segment);
    
    if (m_isWritingToLocalRepo)
    {
      boost::system::error_code ec;
      m_repoSocket.write_some(boost::asio::buffer(segment->wireEncode().wire(), 
                                                  segment->wireEncode().size()), ec);
                                                  
    }
  }
}

size_t
Producer::estimateManifestSize(shared_ptr<Manifest> manifest)
{
  size_t manifestSize = manifest->getName().wireEncode().size();
  
  for (std::list<Name>::const_iterator it = manifest->catalogueBegin(); it != manifest->catalogueEnd(); ++it)
  {
    manifestSize += it->wireEncode().size();
  }
  
  manifestSize += DEFAULT_KEY_LOCATOR_SIZE;
  
  return manifestSize;
}

void
Producer::produce(Data& packet)
{
  if(!m_prefix.isPrefixOf(packet.getName()))
    return;
    
  if (m_onDataInSndBuffer != EMPTY_CALLBACK)
  {
    m_onDataInSndBuffer(*this, packet);
  }
       
  m_sendBuffer.insert(packet);
    
  if (m_onDataLeavesContext != EMPTY_CALLBACK)
  {
    m_onDataLeavesContext(*this, packet);
  }
    
  m_face->put(packet);
  
  if (m_isWritingToLocalRepo)
  {
    boost::system::error_code ec;
    m_repoSocket.write_some(boost::asio::buffer(packet.wireEncode().wire(), 
                                                packet.wireEncode().size()), ec);
  }
  
  // if user requested writing in the remote Repo
  if (!m_targetRepoPrefix.empty())
  {
    repo::RepoCommandParameter commandParameter;
    commandParameter.setName(packet.getName());
  
    Name interestName(m_targetRepoPrefix);
    interestName.append(Name("insert")).append(commandParameter.wireEncode());
  
    Interest repoCommand(interestName);
    m_face->expressInterest(repoCommand,
                          bind(&Producer::onRepoReply, this, _1, _2),
                          bind(&Producer::onRepoTimeout, this, _1));
  }
}

// this can be called either from the thread of the caller
// or from the m_listeningThread
void
Producer::produce(Name suffix, const uint8_t* buf, size_t bufferSize)
{  
  if (bufferSize == 0)
    return;
  
  int bytesPackaged = 0;

  Name name(m_prefix);
  if(!suffix.empty())
  {
    name.append(suffix);
  }
  
  Block nameOnWire = name.wireEncode();
  size_t bytesOccupiedByName = nameOnWire.size();
  
  int signatureSize = 32; //SHA_256 as default
    
  int freeSpaceForContent = m_dataPacketSize - bytesOccupiedByName - signatureSize 
                            - m_keyLocatorSize - DEFAULT_SAFETY_OFFSET;
  
  int numberOfSegments = bufferSize / freeSpaceForContent;
  
  if (numberOfSegments == 0)
    numberOfSegments++;
  
  if (freeSpaceForContent * numberOfSegments < bufferSize)
    numberOfSegments++;
  
  uint64_t currentSegment = 0;  
  uint64_t initialSegment = currentSegment;
  uint64_t finalSegment = currentSegment;
  
  if (m_isMakingManifest) // segmentation with inlined manifests
  {
    shared_ptr<Data> dataSegment;
    shared_ptr<Manifest> manifestSegment;
    bool needManifestSegment = true;
    
    for (int packagedSegments = 0; packagedSegments < numberOfSegments;)
    {
      if (needManifestSegment)
      {
        Name manifestName(m_prefix);
        if (!suffix.empty())
          manifestName.append(suffix);
        manifestName.appendSegment(currentSegment);
        
        if (manifestSegment) // send previous manifest
        {
          manifestSegment->encode();
          passSegmentThroughCallbacks(manifestSegment);
        }
          
        manifestSegment = make_shared<Manifest>(manifestName); // new empty manifest
        manifestSegment->setFinalBlockId(
          name::Component::fromSegment(currentSegment + numberOfSegments - packagedSegments));
        
        finalSegment = currentSegment;
        needManifestSegment = false;
        currentSegment++;
        
        m_keyLocator.clear();
        m_keyLocator.setName(manifestSegment->getName());
      }

      Name fullName(m_prefix);
      if(!suffix.empty())
        fullName.append(suffix);
      fullName.appendSegment(currentSegment);
    
      dataSegment = make_shared<Data>(fullName);
      dataSegment->setFreshnessPeriod(time::milliseconds(m_dataFreshness));
      finalSegment = currentSegment;
      
      if (packagedSegments == numberOfSegments - 1) // last segment
      {
        dataSegment->setContent(&buf[bytesPackaged], bufferSize - bytesPackaged);
        bytesPackaged += bufferSize - bytesPackaged;
      }
      else
      {
        dataSegment->setContent(&buf[bytesPackaged], freeSpaceForContent);
        bytesPackaged += freeSpaceForContent;
      }
      
      dataSegment->setFinalBlockId(
        name::Component::fromSegment(currentSegment + numberOfSegments - packagedSegments - 1));

      passSegmentThroughCallbacks(dataSegment);
      currentSegment++;
      
      size_t manifestSize = estimateManifestSize(manifestSegment);
      size_t fullNameSize = dataSegment->getName().wireEncode().size() 
                            + dataSegment->getSignature().getValue().size();
      
      if (manifestSize + 2*fullNameSize > m_dataPacketSize)
      {
        needManifestSegment = true;
      }
      
      const Block& block = dataSegment->wireEncode();
      ndn::ConstBufferPtr implicitDigest = ndn::crypto::sha256(block.wire(), block.size());
      
      //add implicit digest to the manifest 
      manifestSegment->addNameToCatalogue(
                          dataSegment->getName().getSubName(dataSegment->getName().size() - 1, 1), 
                          implicitDigest
                          );
      
      packagedSegments++;
      
      if (packagedSegments == numberOfSegments) // last manifest to include last segment
      {
        manifestSegment->encode();
        passSegmentThroughCallbacks(manifestSegment);
      }
    }
  }
  else // just normal segmentation
  {
    uint64_t i = 0;
    for (i = currentSegment; i < numberOfSegments + currentSegment; i++)
    {
      Name fullName(m_prefix);
      if(!suffix.empty())
        fullName.append(suffix);
    
      fullName.appendSegment(i);

      shared_ptr<Data> data = make_shared<Data>(fullName);
      data->setFreshnessPeriod(time::milliseconds(m_dataFreshness));
      
      data->setFinalBlockId(name::Component::fromSegment(numberOfSegments + currentSegment - 1));
    
      if (i == numberOfSegments + currentSegment - 1) // last segment
      {
        data->setContent(&buf[bytesPackaged], bufferSize - bytesPackaged);
        bytesPackaged += bufferSize - bytesPackaged;
      }
      else
      {
        data->setContent(&buf[bytesPackaged], freeSpaceForContent);
        bytesPackaged += freeSpaceForContent;
      }
    
      passSegmentThroughCallbacks(data);
    }
  
    finalSegment = i;
  }
  
  // if user requested writing into the REPO
  if (!m_targetRepoPrefix.empty())
  {
    Name dataPrefix(m_prefix);
    dataPrefix.append(suffix);
    writeToRepo(dataPrefix, initialSegment, finalSegment - 1);
  }

  // if data is INFOMAX list or meta info, do not update INFOMAX tree  
  for (unsigned int i=0; i<suffix.size(); i++) {
    if(suffix.get(i).toUri().compare(INFOMAX_INTEREST_TAG) == 0) {
      return;
    }
  }

  // if infomax mode is enabled
  if (m_infomaxType == INFOMAX_MERGE_PRIORITY 
      || m_infomaxType == INFOMAX_SIMPLE_PRIORITY)
  {    
    m_isNewInfomaxData = true;
    size_t lastElement = suffix.size();
    TreeNode *prev = &m_infomaxRoot;  

    for (size_t i = 1; i <= suffix.size() ; ++i) 
    {     
      vector<TreeNode*> prevChildren = prev->getChildren();         
      TreeNode *curr = 0;
      
      for(size_t j=0; j<prevChildren.size(); j++) 
      {
        if(prevChildren[j]->getName().equals(suffix.getSubName(0, i)))
        {
          curr = prevChildren[j];
          break;
        }
      }
    
      if (curr == 0) 
      {
        if (i==lastElement) 
        {          
          curr = new TreeNode(suffix, prev);
          curr->setDataNode(true);
          prev->addChild(curr);                 
        } 
        else 
        {                
          Name *insertName = new Name(suffix.getSubName(0, i).toUri());
          curr = new TreeNode(*insertName, prev);               
          prev->addChild(curr);                 
        }
      }
      
      prev = curr;        
    }
  }
}

void
Producer::asyncProduce(Data& packet)
{
  shared_ptr<Data> p = packet.shared_from_this();
  m_scheduler->scheduleEvent(time::milliseconds(0),
                [p, this] () 
                {
                  produce(*p);
                });
}

void
Producer::asyncProduce(Name suffix, const uint8_t* buffer, size_t bufferSize)
{
  m_scheduler->scheduleEvent( time::milliseconds(0),
                              [suffix, buffer, bufferSize, this] ()
                              {
                                produce(suffix, buffer, bufferSize);
                              }); 
}

void
Producer::writeToRepo(Name dataPrefix, uint64_t startSegment, uint64_t endSegment)
{
  repo::RepoCommandParameter commandParameter;
  commandParameter.setName(dataPrefix);
  commandParameter.setStartBlockId(startSegment);
  commandParameter.setEndBlockId(endSegment);
  
  Name interestName(m_targetRepoPrefix);
  interestName.append(Name("insert")).append(commandParameter.wireEncode());
  
  Interest repoCommand(interestName);
  m_face->expressInterest(repoCommand,
                          bind(&Producer::onRepoReply, this, _1, _2),
                          bind(&Producer::onRepoTimeout, this, _1));
}

void
Producer::onRepoReply(const ndn::Interest& interest, ndn::Data& data)
{
}

void
Producer::onRepoTimeout(const ndn::Interest& interest)
{
}

int
Producer::nack(ApplicationNack nack)
{
  // nack expires faster than good Data packet (10% of lifetime)
  nack.setFreshnessPeriod(time::milliseconds(m_dataFreshness / 10 + 1));

  nack.encode();
  
  if (m_onDataToSecure != EMPTY_CALLBACK)
  {
    m_onDataToSecure(*this, nack);
  }
  else
  {
    m_keyChain.signWithSha256(nack);
  }
    
  // TODO: fix caching strategy
  /*if (m_onDataInSndBuffer != EMPTY_CALLBACK)
  {
    m_onDataInSndBuffer(*nack);
  }*/
    
  //m_sendBuffer.insert(*nack);
    
  if (m_onDataLeavesContext != EMPTY_CALLBACK)
  {
    m_onDataLeavesContext(*this, nack);
  }
  
  //shared_ptr<const Data> dataPtr = nack.shared_from_this(); 
  m_face->put(nack);
  
  return 0;
}

void
Producer::onInterest(const Name& name, const Interest& interest)
{  
  if (m_onInterestEntersContext != EMPTY_CALLBACK)
  {
    m_onInterestEntersContext(*this, interest);
  }

  if (m_receiveBufferSize >= m_receiveBufferCapacity)
  {
    // send Interest NACK
  }
  else
  {
    m_receiveBufferMutex.lock();
    m_receiveBuffer.push(interest.shared_from_this());
    m_receiveBufferSize++;
    m_receiveBufferMutex.unlock();
  }
}

void
Producer::processIncomingInterest(/*const Name& name, const Interest& interest*/)
{
  while (true)
  {
    if (m_receiveBufferSize == 0)
    {
      struct timespec ts;
      ts.tv_sec = 0;
      ts.tv_nsec = 1000000;
      nanosleep(&ts, NULL); // sleep for 1 ms
    }
    else
    {
      m_receiveBufferMutex.lock();
      shared_ptr<const Interest> interest = m_receiveBuffer.front();
      m_receiveBuffer.pop();
      m_receiveBufferSize--;
      m_receiveBufferMutex.unlock();
      
      /*if (m_onInterestToVerify != EMPTY_CALLBACK)
      {
        if (m_onInterestToVerify(const_cast<Interest&>(interest)) == false)
        {
          // produceNACK
        }
      }*/
  
      const Data* data = m_sendBuffer.find(*interest);
      if ((Data*)data != 0)
      {
        if (m_onInterestSatisfiedFromSndBuffer != EMPTY_CALLBACK)
        {
          m_onInterestSatisfiedFromSndBuffer(*this, *interest);
        }
    
        if (m_onDataLeavesContext != EMPTY_CALLBACK)
        {
          m_onDataLeavesContext(*this, *const_cast<Data*>(data));
        }
    
        m_face->put(*data);
      }
      else
      {
        if (m_onInterestProcess != EMPTY_CALLBACK)
        {
          m_onInterestProcess(*this, *interest);
        }
      }
    }
  }

  }

void
Producer::processInterestFromReceiveBuffer()
{
  // put stuff here
}

int
Producer::setContextOption(int optionName, int optionValue)
{
  switch (optionName)
  {
    case DATA_PKT_SIZE:
      if (optionValue < MAX_DATA_PACKET_SIZE && optionValue > 0)
      {
        m_dataPacketSize = optionValue;
        return OPTION_VALUE_SET;
      }
      else
      {
        return OPTION_VALUE_NOT_SET;
      }
    
    case RCV_BUF_SIZE:
      if (optionValue >= 1)
      {
        m_receiveBufferCapacity = optionValue;
        return OPTION_VALUE_SET;
      }
      else
      {
        return OPTION_VALUE_NOT_SET;
      }
    
    case SND_BUF_SIZE:
      if (optionValue >= 0)
      {
        m_sendBuffer.setLimit(optionValue);
        return OPTION_VALUE_SET;
      }
      else
      {
        return OPTION_VALUE_NOT_SET;
      }
      
    case DATA_FRESHNESS:
      m_dataFreshness = optionValue;
      return OPTION_VALUE_SET;
    
    case SIGNATURE_TYPE:
      if (optionValue == OPTION_DEFAULT_VALUE)
        m_signatureType = SHA_256;
      else
        m_signatureType = optionValue;
        
      if (m_signatureType == SHA_256)
        m_signatureSize = 32;
      else if (m_signatureType == RSA_256)
        m_signatureSize = 32;
    
    case INFOMAX_UPDATE_INTERVAL:
      m_infomaxUpdateInterval = optionValue;
      return OPTION_VALUE_SET;
      
    case INFOMAX_PRIORITY:
        m_infomaxType = optionValue;
      
    case INTEREST_ENTER_CNTX:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onInterestEntersContext = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }
  
    case INTEREST_DROP_RCV_BUF:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onInterestDroppedFromRcvBuffer = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }
  
    case INTEREST_PASS_RCV_BUF:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onInterestPassedRcvBuffer = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }
      
    case CACHE_HIT:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onInterestSatisfiedFromSndBuffer = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }
    
    case CACHE_MISS:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onInterestProcess = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }

    case NEW_DATA_SEGMENT:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onNewSegment = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }
  
    case DATA_TO_SECURE:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onDataToSecure = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }
  
    case DATA_IN_SND_BUF:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onDataInSndBuffer = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }
      
    case DATA_LEAVE_CNTX:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onDataLeavesContext = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }
  
    case DATA_EVICT_SND_BUF:
      if (optionValue == EMPTY_CALLBACK)
      {
        m_onDataEvictedFromSndBuffer = EMPTY_CALLBACK;
        return OPTION_VALUE_SET;
      }

    default:
      return OPTION_NOT_FOUND;
  }
}
  
int
Producer::setContextOption(int optionName, bool optionValue)
{
  switch (optionName)
  {    
    case FAST_SIGNING:
      m_isMakingManifest = optionValue;
      return OPTION_VALUE_SET;
  
    case LOCAL_REPO:
    
      if (optionValue == true)
      {
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::from_string("127.0.0.1"), 1000);
        boost::system::error_code ec;
        m_repoSocket.connect(ep,ec);

        if (ec)
        {
          return OPTION_VALUE_NOT_SET;
        }
      }  
      
      m_isWritingToLocalRepo = optionValue;
      return OPTION_VALUE_SET;
  
    case INFOMAX:
      if (optionValue == true)
      {
        m_infomaxPrioritizer = make_shared<Prioritizer>(this);
        m_infomaxType = INFOMAX_SIMPLE_PRIORITY;
        //updateInfomaxTree();
        m_scheduler->scheduleEvent( time::milliseconds(m_infomaxUpdateInterval), 
                                bind(&Producer::updateInfomaxTree, this));
      }
      else
      {
        m_infomaxType = INFOMAX_NONE;
      }
  
    default:
      return OPTION_NOT_FOUND;
  }
}
  
int
Producer::setContextOption(int optionName, Name optionValue)
{
  switch (optionName)
  {
    case PREFIX:
      m_prefix = optionValue;
      return OPTION_VALUE_SET;
  
    case REMOTE_REPO_PREFIX:
      m_targetRepoPrefix = optionValue;
      return OPTION_VALUE_SET;
  
    case FORWARDING_STRATEGY:
      m_forwardingStrategy = optionValue;
      if (m_forwardingStrategy.empty())
      {
        nfd::ControlParameters parameters;
        parameters.setName(m_prefix);

        m_controller->start<nfd::StrategyChoiceUnsetCommand>(parameters,
                                                 bind(&Producer::onStrategyChangeSuccess, this, _1,
                                                      "Successfully unset strategy choice"),
                                                 bind(&Producer::onStrategyChangeError, this, _1, _2,
                                                      "Failed to unset strategy choice"));
      }
      else
      {
        nfd::ControlParameters parameters;
        parameters
          .setName(m_prefix)
          .setStrategy(m_forwardingStrategy);

        m_controller->start<nfd::StrategyChoiceSetCommand>(parameters,
                                               bind(&Producer::onStrategyChangeSuccess, this, _1,
                                                    "Successfully set strategy choice"),
                                               bind(&Producer::onStrategyChangeError, this, _1, _2,
                                                    "Failed to set strategy choice"));

      }
      return OPTION_VALUE_SET;
  
    default:
      return OPTION_VALUE_NOT_SET;
  }
}

int
Producer::setContextOption(int optionName, ProducerDataCallback optionValue)
{
  switch (optionName)
  {
    case NEW_DATA_SEGMENT:
      m_onNewSegment = optionValue;
      return OPTION_VALUE_SET;
  
    case DATA_TO_SECURE:
      m_onDataToSecure = optionValue;
      return OPTION_VALUE_SET;
  
    case DATA_IN_SND_BUF:
      m_onDataInSndBuffer = optionValue;
      return OPTION_VALUE_SET;
      
    case DATA_LEAVE_CNTX:
      m_onDataLeavesContext = optionValue;
      return OPTION_VALUE_SET;
  
    case DATA_EVICT_SND_BUF:
      m_onDataEvictedFromSndBuffer = optionValue;
      return OPTION_VALUE_SET;
      
    default:
      return OPTION_NOT_FOUND;
  }
}

int
Producer::setContextOption(int optionName, ProducerInterestCallback optionValue)
{
  switch (optionName)
  {
    case INTEREST_ENTER_CNTX:
      m_onInterestEntersContext = optionValue;
      return OPTION_VALUE_SET;
  
    case INTEREST_DROP_RCV_BUF:
      m_onInterestDroppedFromRcvBuffer = optionValue;
      return OPTION_VALUE_SET;
  
    case INTEREST_PASS_RCV_BUF:
      m_onInterestPassedRcvBuffer = optionValue;
      return OPTION_VALUE_SET;
      
    case CACHE_HIT:
      m_onInterestSatisfiedFromSndBuffer = optionValue;
      return OPTION_VALUE_SET;
    
    case CACHE_MISS:
      m_onInterestProcess = optionValue;
      return OPTION_VALUE_SET;
  
    default:
      return OPTION_NOT_FOUND;
  }
}

int
Producer::setContextOption(int optionName, ConsumerDataCallback optionValue)
{
  return OPTION_NOT_FOUND;
}

int
Producer::setContextOption(int optionName, ConsumerDataVerificationCallback optionValue)
{
  return OPTION_NOT_FOUND;
}

int
Producer::setContextOption(int optionName, ConsumerInterestCallback optionValue)
{
  return OPTION_NOT_FOUND;
}


int
Producer::setContextOption(int optionName, ConsumerContentCallback optionValue)
{
  return OPTION_NOT_FOUND;
}

int
Producer::setContextOption(int optionName, ConsumerNackCallback optionValue)
{
  return OPTION_NOT_FOUND;
}

int
Producer::setContextOption(int optionName, ConsumerManifestCallback optionValue)
{
  return OPTION_NOT_FOUND;
}

int
Producer::setContextOption(int optionName, KeyLocator optionValue)
{
  return OPTION_NOT_FOUND;
}
  
int
Producer::setContextOption(int optionName, Exclude optionValue)
{
  return OPTION_NOT_FOUND;
}


int
Producer::getContextOption(int optionName, int& optionValue)
{
  switch (optionName)
  { 
    case RCV_BUF_SIZE:
      optionValue = m_receiveBufferCapacity;
      return OPTION_FOUND;
  
    case SND_BUF_SIZE:
      optionValue = m_sendBuffer.getLimit();
      return OPTION_FOUND;
  
    case DATA_PKT_SIZE:
      optionValue = m_dataPacketSize;
      return OPTION_FOUND;
      
    case DATA_FRESHNESS:
      optionValue = m_dataFreshness;
      return OPTION_FOUND;
    
    case SIGNATURE_TYPE:
      optionValue = m_signatureType;
      return OPTION_FOUND;
    
    case REGISTRATION_STATUS:
      optionValue = m_registrationStatus;
      return OPTION_FOUND;
    
    case INFOMAX_PRIORITY:
      optionValue = m_infomaxType;
      return OPTION_FOUND;
    
    case INFOMAX_UPDATE_INTERVAL:
      optionValue = m_infomaxUpdateInterval;
      return OPTION_FOUND;
    
    default:
      return OPTION_NOT_FOUND;
  }
}
  
int
Producer::getContextOption(int optionName, bool& optionValue)
{
  switch (optionName)
  {    
    case FAST_SIGNING:
      optionValue = m_isMakingManifest;
      return OPTION_FOUND;
  
    case LOCAL_REPO:
      optionValue = m_isWritingToLocalRepo;
      return OPTION_FOUND;
  
    case INFOMAX:
      if (m_infomaxType == INFOMAX_SIMPLE_PRIORITY || m_infomaxType == INFOMAX_MERGE_PRIORITY)
      {
        optionValue = true;
      }
      else
      {
        optionValue = false;
      }
      
      return OPTION_FOUND;
  
    default:
      return OPTION_NOT_FOUND;
  }
}
  
int
Producer::getContextOption(int optionName, Name& optionValue)
{
  switch (optionName)
  {   
    case PREFIX:
      optionValue = m_prefix;
      return OPTION_FOUND;
    
    case REMOTE_REPO_PREFIX:
      optionValue = m_targetRepoPrefix;
      return OPTION_FOUND;
    
    case FORWARDING_STRATEGY:
      optionValue = m_forwardingStrategy;
      return OPTION_FOUND;
    
    default:
      return OPTION_NOT_FOUND;
  }
}

int
Producer::getContextOption(int optionName, ProducerDataCallback& optionValue)
{
  switch (optionName)
  {
    case NEW_DATA_SEGMENT:
      optionValue = m_onNewSegment;
      return OPTION_FOUND;
  
    case DATA_TO_SECURE:
      optionValue = m_onDataToSecure;
      return OPTION_FOUND;
  
    case DATA_IN_SND_BUF:
      optionValue = m_onDataInSndBuffer;
      return OPTION_FOUND;
    
    case DATA_LEAVE_CNTX:
      optionValue = m_onDataLeavesContext;
      return OPTION_FOUND;
  
    case DATA_EVICT_SND_BUF:
      optionValue = m_onDataEvictedFromSndBuffer;
      return OPTION_FOUND;
    
    default:
      return OPTION_NOT_FOUND;
  }
}

int
Producer::getContextOption(int optionName, ProducerInterestCallback& optionValue)
{
  switch (optionName)
  {
    case INTEREST_ENTER_CNTX:
      optionValue = m_onInterestEntersContext;
      return OPTION_FOUND;
  
    case INTEREST_DROP_RCV_BUF:
      optionValue = m_onInterestDroppedFromRcvBuffer;
      return OPTION_FOUND;
  
    case INTEREST_PASS_RCV_BUF:
      optionValue = m_onInterestPassedRcvBuffer;
      return OPTION_FOUND;
      
    case CACHE_HIT:
      optionValue = m_onInterestSatisfiedFromSndBuffer;
      return OPTION_FOUND;
    
    case CACHE_MISS:
      optionValue = m_onInterestProcess;
      return OPTION_FOUND;
    
    default:
      return OPTION_NOT_FOUND;
  }
}

int
Producer::getContextOption(int optionName, ConsumerDataCallback& optionValue)
{
  return OPTION_NOT_FOUND;
}

int
Producer::getContextOption(int optionName, ConsumerDataVerificationCallback& optionValue)
{
  return OPTION_NOT_FOUND;
} 

int
Producer::getContextOption(int optionName, ConsumerInterestCallback& optionValue)
{
  return OPTION_NOT_FOUND;
}

int
Producer::getContextOption(int optionName, ConsumerContentCallback& optionValue)
{
  return OPTION_NOT_FOUND;
}

int
Producer::getContextOption(int optionName, ConsumerNackCallback& optionValue)
{
  return OPTION_NOT_FOUND;
}

int
Producer::getContextOption(int optionName, ConsumerManifestCallback& optionValue)
{
  return OPTION_NOT_FOUND;
}

int
Producer::setContextOption(int optionName, size_t optionValue)
{
  switch (optionName) 
  {
    case RCV_BUF_SIZE:
      if (m_receiveBufferCapacity >= 1)
      {
        m_receiveBufferCapacity = optionValue;
        return OPTION_VALUE_SET;
      }
  
    default:
      return OPTION_VALUE_NOT_SET;
  }
}

int
Producer::getContextOption(int optionName, size_t& optionValue)
{
  switch (optionName)
  {
    case RCV_BUF_SIZE:
      optionValue = m_receiveBufferCapacity;
      return OPTION_FOUND;
    
    case SND_BUF_SIZE:
      optionValue = m_sendBuffer.size();
      return OPTION_FOUND;
    
    default:
      return OPTION_NOT_FOUND;
  }
}

int
Producer::getContextOption(int optionName, KeyLocator& optionValue)
{
  return OPTION_NOT_FOUND;
}
  
int
Producer::getContextOption(int optionName, Exclude& optionValue)
{
  return OPTION_NOT_FOUND;
}

int
Producer::getContextOption(int optionName, shared_ptr<Face>& optionValue)
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
Producer::getContextOption(int optionName, TreeNode& optionValue)
{
  switch (optionName)
  {
    case INFOMAX_ROOT:
      optionValue = m_infomaxRoot;
      return OPTION_FOUND;
    
    default: return OPTION_NOT_FOUND;
  }
}

void
Producer::onStrategyChangeSuccess(const nfd::ControlParameters& commandSuccessResult, 
                                  const std::string& message)
{
}

void
Producer::onStrategyChangeError(uint32_t code, const std::string& error, const std::string& message)
{
}


} //namespace ndn

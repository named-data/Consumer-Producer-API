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

#include "infomax-data-retrieval.hpp"

namespace ndn {

InfoMaxDataRetrieval::InfoMaxDataRetrieval(Context* context)
  : DataRetrievalProtocol(context)
  , m_requestVersion(1)
  , m_requestListNum(1)
  , m_isInit(true)
{
}

InfoMaxDataRetrieval::~InfoMaxDataRetrieval()
{
  m_infoMaxList.clear();
  stop();
}

void
InfoMaxDataRetrieval::processInfoMaxPayload(Consumer& c, const uint8_t* buffer, size_t bufferSize)
{
  std::string content((char*)buffer, bufferSize);

  // std::cout << "REASSEMBLED " << content << std::endl;
  // std::cout << "Size " << bufferSize << std::endl;

  convertStringToList(content);
}

void
InfoMaxDataRetrieval::processInfoMaxData(Consumer& c, const Data& data)
{
  // std::cout << "LIST IN CNTX" << std::endl;
}

void
InfoMaxDataRetrieval::processLeavingInfoMaxInterest(Consumer& c, Interest& interest)
{
  // std::cout << "INFOMAX INTEREST LEAVES " << interest.toUri() << std::endl;
}

void
InfoMaxDataRetrieval::processInfoMaxInitPayload(Consumer& c, const uint8_t* buffer, size_t bufferSize)
{
  // Fetching the latest version number and the total number of lists
  std::string content((char*)buffer, bufferSize);

  std::vector<std::string> metaInfo;
  std::string buf;
  std::stringstream ss(content);

  while (ss >> buf)
    metaInfo.push_back(buf);

  m_requestVersion = std::stoi(metaInfo[0]);
  m_maxListNum = std::stoi(metaInfo[1]);
}

void
InfoMaxDataRetrieval::processInfoMaxInitData(Consumer& c, const Data& data)
{
  // std::cout << "METAINFO IN CNTX" << std::endl;
}

void
InfoMaxDataRetrieval::processLeavingInfoMaxInitInterest(Consumer& c, Interest& interest)
{
  // std::cout << "INFOMAX INIT INTEREST LEAVES " << interest.toUri() << std::endl;
}

void
InfoMaxDataRetrieval::start()
{
  m_rdr = make_shared<ReliableDataRetrieval>(m_context);

  if (m_isInit) {
    // Reqeust version number and the total number of lists (/prefix/InfoMax/MetaInfo)
    m_isInit = false;

    ConsumerInterestCallback processLeavingInterest;
    ConsumerDataCallback processData;
    ConsumerContentCallback processPayload;
    m_context->getContextOption(INTEREST_LEAVE_CNTX, processLeavingInterest);
    m_context->getContextOption(DATA_ENTER_CNTX, processData);
    m_context->getContextOption(CONTENT_RETRIEVED, processPayload);

    m_context->setContextOption(MUST_BE_FRESH_S, true);
    m_context->setContextOption(INTEREST_LEAVE_CNTX,
                                (ConsumerInterestCallback)bind(&InfoMaxDataRetrieval::processLeavingInfoMaxInitInterest, this, _1, _2));
    m_context->setContextOption(DATA_ENTER_CNTX, (ConsumerDataCallback)bind(&InfoMaxDataRetrieval::processInfoMaxInitData, this, _1, _2));
    m_context->setContextOption(CONTENT_RETRIEVED,
                                (ConsumerContentCallback)bind(&InfoMaxDataRetrieval::processInfoMaxInitPayload, this, _1, _2, _3));

    Name infomaxInitSuffix(INFOMAX_INTEREST_TAG);
    infomaxInitSuffix.append(Name(INFOMAX_META_INTEREST_TAG));
    m_context->setContextOption(SUFFIX, infomaxInitSuffix);
    m_rdr->start();

    m_context->setContextOption(INTEREST_LEAVE_CNTX, processLeavingInterest);
    m_context->setContextOption(DATA_ENTER_CNTX, processData);
    m_context->setContextOption(CONTENT_RETRIEVED, processPayload);
  }

  if (m_infoMaxList.empty()) {
    if (m_requestListNum > m_maxListNum) {
      // cout << "All data fetched" << endl;
      return;
    }

    // If current list is empty, issue InfoMax interest to fetch new list
    ConsumerInterestCallback processLeavingInterest;
    ConsumerDataCallback processData;
    ConsumerContentCallback processPayload;
    m_context->getContextOption(INTEREST_LEAVE_CNTX, processLeavingInterest);
    m_context->getContextOption(DATA_ENTER_CNTX, processData);
    m_context->getContextOption(CONTENT_RETRIEVED, processPayload);

    Name infomaxSuffix(INFOMAX_INTEREST_TAG);
    infomaxSuffix.appendNumber(m_requestVersion);
    infomaxSuffix.appendNumber(m_requestListNum++);

    m_context->setContextOption(MUST_BE_FRESH_S, true);
    m_context->setContextOption(INTEREST_LEAVE_CNTX,
                                (ConsumerInterestCallback)bind(&InfoMaxDataRetrieval::processLeavingInfoMaxInterest, this, _1, _2));
    m_context->setContextOption(DATA_ENTER_CNTX, (ConsumerDataCallback)bind(&InfoMaxDataRetrieval::processInfoMaxData, this, _1, _2));
    m_context->setContextOption(CONTENT_RETRIEVED,
                                (ConsumerContentCallback)bind(&InfoMaxDataRetrieval::processInfoMaxPayload, this, _1, _2, _3));
    m_context->setContextOption(SUFFIX, infomaxSuffix);

    m_context->setContextOption(RUNNING, false);
    m_rdr->start();

    m_context->setContextOption(INTEREST_LEAVE_CNTX, processLeavingInterest);
    m_context->setContextOption(DATA_ENTER_CNTX, processData);
    m_context->setContextOption(CONTENT_RETRIEVED, processPayload);
  }

  m_context->setContextOption(RUNNING, false);
  m_context->setContextOption(SUFFIX, *(m_infoMaxList.front()));
  m_rdr->start();
  m_infoMaxList.pop_front();
}

void
InfoMaxDataRetrieval::convertStringToList(std::string& names)
{
  m_infoMaxList.clear();
  std::string buf;
  std::stringstream ss(names);

  while (ss >> buf)
    m_infoMaxList.push_back(make_shared<Name>(buf));
}

void
InfoMaxDataRetrieval::stop()
{
  m_rdr->stop();
}

} //namespace ndn

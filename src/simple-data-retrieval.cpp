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

#include "simple-data-retrieval.hpp"
#include "consumer-context.hpp"

namespace ndn {

SimpleDataRetrieval::SimpleDataRetrieval(Context* context)
  : DataRetrievalProtocol(context)
{
  context->getContextOption(FACE, m_face);
}

void
SimpleDataRetrieval::start()
{
  m_isRunning = true;
  sendInterest();
}

void
SimpleDataRetrieval::sendInterest()
{
  Name prefix;
  m_context->getContextOption(PREFIX, prefix);

  Name suffix;
  m_context->getContextOption(SUFFIX, suffix);

  if (!suffix.empty()) {
    prefix.append(suffix);
  }

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

  m_face->expressInterest(interest,
                          bind(&SimpleDataRetrieval::onData, this, _1, _2),
                          bind(&SimpleDataRetrieval::onNack, this, _1, _2),
                          bind(&SimpleDataRetrieval::onTimeout, this, _1));

  bool isAsync = false;
  m_context->getContextOption(ASYNC_MODE, isAsync);

  if (!isAsync) {
    m_face->processEvents();
  }
}

void
SimpleDataRetrieval::stop()
{
  m_isRunning = false;
}

void
SimpleDataRetrieval::onData(const ndn::Interest& interest, const ndn::Data& data)
{
  if (m_isRunning == false)
    return;

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
  if (onDataToVerify != EMPTY_CALLBACK) {
    if (onDataToVerify(*dynamic_cast<Consumer*>(m_context), data) == true) // runs verification routine
    {
      const Block content = data.getContent();

      ConsumerContentCallback onPayload = EMPTY_CALLBACK;
      m_context->getContextOption(CONTENT_RETRIEVED, onPayload);
      if (onPayload != EMPTY_CALLBACK) {
        onPayload(*dynamic_cast<Consumer*>(m_context), content.value(), content.value_size());
      }
    }
  }
  else {
    const Block content = data.getContent();

    ConsumerContentCallback onPayload = EMPTY_CALLBACK;
    m_context->getContextOption(CONTENT_RETRIEVED, onPayload);
    if (onPayload != EMPTY_CALLBACK) {
      onPayload(*dynamic_cast<Consumer*>(m_context), content.value(), content.value_size());
    }
  }

  m_isRunning = false;
}

void
SimpleDataRetrieval::onNack(const Interest& interest, const lp::Nack& nack)
{
  if (m_isRunning == false)
    return;

  ConsumerInterestCallback onInterestExpired = EMPTY_CALLBACK;
  m_context->getContextOption(INTEREST_EXPIRED, onInterestExpired);
  if (onInterestExpired != EMPTY_CALLBACK) {
    onInterestExpired(*dynamic_cast<Consumer*>(m_context), const_cast<Interest&>(interest));
  }

  m_isRunning = false;
}

void
SimpleDataRetrieval::onTimeout(const ndn::Interest& interest)
{
  if (m_isRunning == false)
    return;

  ConsumerInterestCallback onInterestExpired = EMPTY_CALLBACK;
  m_context->getContextOption(INTEREST_EXPIRED, onInterestExpired);
  if (onInterestExpired != EMPTY_CALLBACK) {
    onInterestExpired(*dynamic_cast<Consumer*>(m_context), const_cast<Interest&>(interest));
  }

  m_isRunning = false;
}

} //namespace ndn

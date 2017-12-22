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

#ifndef UNRELIABLE_DATA_RETRIEVAL_HPP
#define UNRELIABLE_DATA_RETRIEVAL_HPP

#include "data-retrieval-protocol.hpp"
#include "selector-helper.hpp"

namespace ndn {

/*
 * UDR provides unreliable and unordered delivery of data segments that belong to a single ADU
 * between the consumer application and the NDN network.
 * In UDR, the transfer of every ADU begins with the segment number zero.
 * UDR infers the name of the last data segment of the sequence with help of FinalBlockID field,
 * and stops the transmission of Interest packets at this name (segment).
 * FinalBlockID packet field is set at the moment of application frame (ADU) segmentation.
 */
class UnreliableDataRetrieval : public DataRetrievalProtocol
{
public:
  UnreliableDataRetrieval(Context* context);

  void
  start();

  void
  stop();

private:
  void
  sendInterest();

  void
  onData(const Interest& interest, const Data& data);

  void
  onNack(const Interest& interest, const lp::Nack& data);

  void
  onTimeout(const Interest& interest);

  void
  checkFastRetransmissionConditions(const Interest& interest);

  void
  fastRetransmit(const Interest& interest, uint64_t segNumber);

  void
  removeAllPendingInterests();

private:
  bool m_isFinalBlockNumberDiscovered;
  int m_nTimeouts;

  uint64_t m_finalBlockNumber;
  uint64_t m_segNumber;

  int m_currentWindowSize;
  int m_interestsInFlight;

  std::unordered_map<uint64_t, const PendingInterestId*> m_expressedInterests; // by segment number

  // Fast Retransmission
  std::map<uint64_t, bool> m_receivedSegments;
  std::map<uint64_t, bool> m_fastRetxSegments;
};

} // namespace ndn

#endif // UNRELIABLE_DATA_RETRIEVAL_HPP

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

#ifndef RELIABLE_DATA_RETRIEVAL_HPP
#define RELIABLE_DATA_RETRIEVAL_HPP

#include "data-retrieval-protocol.hpp"
#include "rtt-estimator.hpp"
#include "selector-helper.hpp"

namespace ndn {

/*
 * Two types of packet losses are possible in NDN, leading to a situation when NDN application
 * begins to speculate about possible reasons of failed Interest/Data exchange:
 * 1) the Interest was lost in transit before it reached the data, which may reside in cache,
 * or needs to be produced;
 * 2) the Interest reached the producer-application and the application did not respond;
 * 3) returning Data packet was lost;
 * 4) returning Data packet could not be validated by its signature.
 *
 * Reliable Data Retrieval protocol (RDR) uses Interest retransmission and negative acknowledgements
 * to handle the packet losses mentioned above. Interest retransmission is activated if
 * the expressed Interest packet is not satisfied when it times out, and in case
 * the negative acknowledgment carrying Retry-After field was retrieved instead of the actual data.
 *
 * Another type of transmission errors in NDN network is a failure of Data verification.
 * Data verification error can be caused by packet tampering, content poisoning by a
 * non-credible publisher, expired public key of a credible publisher, and other possible cases
 * depending on the selected trust model. While Data verification operation is performed separately
 * by a security part of the library, Data retrieval protocol will make an attempt to recover from
 * this type of error.
 *
 * To recover from the Data verification failure, RDR performs retransmission of Interest packet
 * with Exclude selector set to exclude any possible Data packet having the same name and the digest
 * (e.g. hash, checksum) of the packet that has failed verification. RDR limits its exclude selector
 * to five digests, which means that the protocol attempts up to five retransmissions in order to
 * recover from the Data verification failure.
 */
class ReliableDataRetrieval : public DataRetrievalProtocol
{
public:
  ReliableDataRetrieval(Context* context);

  ~ReliableDataRetrieval();

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
  onNack(const Interest& interest, const lp::Nack& nack);

  void
  onTimeout(const Interest& interest);

  void
  onManifestData(const Interest& interest, const Data& data);

  void
  onNackData(const Interest& interest, const Data& data);

  void
  onContentData(const Interest& interest, const Data& data);

  void
  reassemble();

  void
  copyContent(const Data& data);

  bool
  referencesManifest(const Data& data);

  void
  retransmitFreshInterest(const Interest& interest);

  bool
  retransmitInterestWithExclude(const Interest& interest, const Data& dataSegment);

  bool
  retransmitInterestWithDigest(const Interest& interest, const Data& dataSegment, const Manifest& manifestSegment);

  bool
  verifySegmentWithManifest(const Manifest& manifestSegment, const Data& dataSegment);

  name::Component
  getDigestFromManifest(const Manifest& manifestSegment, const Data& dataSegment);

  void
  checkFastRetransmissionConditions(const Interest& interest);

  void
  fastRetransmit(const Interest& interest, uint64_t segNumber);

  void
  removeAllPendingInterests();

  void
  removeAllScheduledInterests();

  void
  paceInterests(int nInterests, time::milliseconds timeWindow);

private:
  Scheduler* m_scheduler;
  KeyChain m_keyChain;

  // reassembly variables
  bool m_isFinalBlockNumberDiscovered;
  uint64_t m_finalBlockNumber;
  uint64_t m_lastReassembledSegment;
  std::vector<uint8_t> m_contentBuffer;
  size_t m_contentBufferSize;

  // transmission variables
  int m_currentWindowSize;
  int m_interestsInFlight;
  uint64_t m_segNumber;
  std::unordered_map<uint64_t, int> m_interestRetransmissions;                       // by segment number
  std::unordered_map<uint64_t, const PendingInterestId*> m_expressedInterests;       // by segment number
  std::unordered_map<uint64_t, EventId> m_scheduledInterests;                        // by segment number
  std::unordered_map<uint64_t, time::steady_clock::time_point> m_interestTimepoints; // by segment
  RttEstimator m_rttEstimator;

  // buffers
  std::map<uint64_t, shared_ptr<const Data>> m_receiveBuffer;         // verified segments by segment number
  std::map<uint64_t, shared_ptr<const Data>> m_unverifiedSegments;    // used with embedded manifests
  std::map<uint64_t, shared_ptr<const Manifest>> m_verifiedManifests; // by segment number

  // Fast Retransmission
  std::map<uint64_t, bool> m_receivedSegments;
  std::unordered_map<uint64_t, bool> m_fastRetxSegments;
};

} // namespace ndn

#endif // RELIABLE_DATA_RETRIEVAL_HPP

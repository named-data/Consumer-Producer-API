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

// correct way to include Consumer/Producer API headers
//#include <Consumer-Producer-API/producer-context.hpp>
#include "consumer-context.hpp"
#include "producer-context.hpp"

#include <ndn-cxx/security/signing-helpers.hpp>
#include <ndn-cxx/util/time.hpp>

#include <iostream>

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions
namespace examples {

#define CONTENT_LENGTH 1 * 1024 * 1024
#define IDENTITY_NAME "/sequence/performance"

class Performance
{
public:
  Performance()
  {
  }

  void
  onNewSegment(Producer& p, Data& data)
  {
    if (data.getName().get(-1).toSegment() == 0) {
      m_segmentationStart = time::system_clock::now();
    }
  }

  void
  onSegmentFinalized(Producer& p, Data& data)
  {
    m_segmentationStop = time::system_clock::now();
  }

  ndn::time::steady_clock::TimePoint::clock::duration
  getSegmentationDuration()
  {
    return m_segmentationStop - m_segmentationStart;
  }

  void
  onInterest(Producer& p, const Interest& interest)
  {
    std::cout << "Entering " << interest.toUri() << std::endl;
  }

private:
  time::system_clock::TimePoint m_segmentationStart;
  time::system_clock::TimePoint m_segmentationStop;
};

class Signer
{
public:
  Signer()
    : m_counter(0)
    , m_identityName(IDENTITY_NAME)
  {
    m_keyChain.createIdentity(m_identityName);
  }

  void
  onPacket(Producer& p, Data& data)
  {
    m_counter++;
    m_keyChain.sign(data, signingByIdentity(m_identityName));
  }

private:
  KeyChain m_keyChain;
  int m_counter;
  Name m_identityName;
};

int
main(int argc, char** argv)
{
  Signer signer;
  Performance performance;

  Name sampleName("/a/b/c");

  Producer p(sampleName);
  p.setContextOption(SND_BUF_SIZE, 60000);

  p.setContextOption(NEW_DATA_SEGMENT, (ProducerDataCallback)bind(&Performance::onNewSegment, &performance, _1, _2));

  p.setContextOption(DATA_TO_SECURE, (ProducerDataCallback)bind(&Signer::onPacket, &signer, _1, _2));

  p.setContextOption(DATA_LEAVE_CNTX, (ProducerDataCallback)bind(&Performance::onSegmentFinalized, &performance, _1, _2));

  p.setContextOption(INTEREST_ENTER_CNTX, (ProducerInterestCallback)bind(&Performance::onInterest, &performance, _1, _2));

  p.attach();

  uint8_t* content = new uint8_t[CONTENT_LENGTH];
  p.produce(Name(), content, CONTENT_LENGTH);

  std::cout << "**************************************************************" << std::endl;
  std::cout << "Sequence segmentation duration " << performance.getSegmentationDuration() << std::endl;

  sleep(500);

  return 0;
}

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
  return ndn::examples::main(argc, argv);
}

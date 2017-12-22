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
//#include <Consumer-Producer-API/consumer-context.hpp>
#include "consumer-context.hpp"
#include "producer-context.hpp"

#include <ndn-cxx/security/verification-helpers.hpp>
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
    : m_byteCounter(0)
  {
  }

  void
  onInterestLeaves(Consumer& c, Interest& interest)
  {
    std::cout << "Leaving: " << interest.toUri() << std::endl;
  }

  void
  onDataEnters(Consumer& c, const Data& data)
  {
    std::cout << "DATA IN" << data.getName() << std::endl;
    if (data.getName().get(-1).toSegment() == 0) {
      m_reassemblyStart = time::system_clock::now();
    }
  }

  void
  onContent(Consumer& c, const uint8_t* buffer, size_t bufferSize)
  {
    m_byteCounter += bufferSize;

    if (m_byteCounter == CONTENT_LENGTH) {
      std::cout << "DONE" << std::endl;
      m_reassemblyStop = time::system_clock::now();
    }
  }

  ndn::time::steady_clock::TimePoint::clock::duration
  getReassemblyDuration()
  {
    return m_reassemblyStop - m_reassemblyStart;
  }

private:
  uint32_t m_byteCounter;
  time::system_clock::TimePoint m_reassemblyStart;
  time::system_clock::TimePoint m_reassemblyStop;
};

class Verificator
{
public:
  Verificator()
  {
    m_key = m_keyChain.getPib().getIdentity(IDENTITY_NAME).getDefaultKey();
  };

  bool
  onPacket(Consumer& c, const Data& data)
  {
    if (security::verifySignature(data, m_key)) {
      std::cout << "VERIFIED " << data.getName() << std::endl;
      return true;
    }
    else {
      std::cout << "UNVERIFIED " << data.getName() << std::endl;
      return false;
    }
  }

private:
  KeyChain m_keyChain;
  security::Key m_key;
};


int
main(int argc, char** argv)
{
  Verificator verificator;
  Performance performance;

  Name sampleName("/a/b/c");

  Consumer c(sampleName, RDR);

  c.setContextOption(MUST_BE_FRESH_S, true);

  c.setContextOption(DATA_ENTER_CNTX, (ConsumerDataCallback)bind(&Performance::onDataEnters, &performance, _1, _2));

  c.setContextOption(INTEREST_LEAVE_CNTX, (ConsumerInterestCallback)bind(&Performance::onInterestLeaves, &performance, _1, _2));

  c.setContextOption(DATA_TO_VERIFY, (ConsumerDataVerificationCallback)bind(&Verificator::onPacket, &verificator, _1, _2));

  c.setContextOption(CONTENT_RETRIEVED, (ConsumerContentCallback)bind(&Performance::onContent, &performance, _1, _2, _3));

  c.consume(Name());

  std::cout << "**************************************************************" << std::endl;
  std::cout << "Sequence reassembly duration " << performance.getReassemblyDuration() << std::endl;

  return 0;
}

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
  return ndn::examples::main(argc, argv);
}

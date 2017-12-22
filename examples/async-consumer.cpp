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

#include <iostream>

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions
namespace examples {

class CallbackContainer
{
public:
  CallbackContainer()
  {
  }

  void
  processPayload(Consumer& c, const uint8_t* buffer, size_t bufferSize)
  {
    std::string content1((char*)buffer, bufferSize);

    Name prefix;
    c.getContextOption(PREFIX, prefix);
    Name suffix;
    c.getContextOption(SUFFIX, suffix);

    std::cout << "CONTENT for " << prefix << suffix << std::endl;
  }

  void
  processData(Consumer& c, const Data& data)
  {
    std::cout << "DATA " << data.getName() << std::endl;
  }

  void
  processLeavingInterest(Consumer& c, Interest& interest)
  {
    std::cout << "LEAVES  " << interest.toUri() << std::endl;
  }
};

int
main(int argc, char** argv)
{
  CallbackContainer stubs;

  Consumer c1(Name("/q/w/e"), RDR);
  c1.setContextOption(MUST_BE_FRESH_S, true);
  //c1.setContextOption(FORWARDING_STRATEGY, BROADCAST);

  c1.setContextOption(INTEREST_LEAVE_CNTX, (ConsumerInterestCallback)bind(&CallbackContainer::processLeavingInterest, &stubs, _1, _2));

  c1.setContextOption(DATA_ENTER_CNTX, (ConsumerDataCallback)bind(&CallbackContainer::processData, &stubs, _1, _2));

  c1.setContextOption(CONTENT_RETRIEVED, (ConsumerContentCallback)bind(&CallbackContainer::processPayload, &stubs, _1, _2, _3));

  Consumer c2(Name("/t/y/u"), RDR);
  c2.setContextOption(MUST_BE_FRESH_S, true);
  //c2.setContextOption(FORWARDING_STRATEGY, BROADCAST);

  c2.setContextOption(INTEREST_LEAVE_CNTX, (ConsumerInterestCallback)bind(&CallbackContainer::processLeavingInterest, &stubs, _1, _2));

  c2.setContextOption(DATA_ENTER_CNTX, (ConsumerDataCallback)bind(&CallbackContainer::processData, &stubs, _1, _2));

  c2.setContextOption(CONTENT_RETRIEVED, (ConsumerContentCallback)bind(&CallbackContainer::processPayload, &stubs, _1, _2, _3));

  Consumer c3(Name("/a/s/d"), RDR);
  c3.setContextOption(MUST_BE_FRESH_S, true);
  //c3.setContextOption(FORWARDING_STRATEGY, BROADCAST);

  c3.setContextOption(INTEREST_LEAVE_CNTX, (ConsumerInterestCallback)bind(&CallbackContainer::processLeavingInterest, &stubs, _1, _2));

  c3.setContextOption(DATA_ENTER_CNTX, (ConsumerDataCallback)bind(&CallbackContainer::processData, &stubs, _1, _2));

  c3.setContextOption(CONTENT_RETRIEVED, (ConsumerContentCallback)bind(&CallbackContainer::processPayload, &stubs, _1, _2, _3));

  Consumer c4(Name("/g/h/j"), RDR);
  c4.setContextOption(MUST_BE_FRESH_S, true);
  //c4.setContextOption(FORWARDING_STRATEGY, BROADCAST);

  c4.setContextOption(INTEREST_LEAVE_CNTX, (ConsumerInterestCallback)bind(&CallbackContainer::processLeavingInterest, &stubs, _1, _2));

  c4.setContextOption(DATA_ENTER_CNTX, (ConsumerDataCallback)bind(&CallbackContainer::processData, &stubs, _1, _2));

  c4.setContextOption(CONTENT_RETRIEVED, (ConsumerContentCallback)bind(&CallbackContainer::processPayload, &stubs, _1, _2, _3));

  Consumer c5(Name("/b/n/m"), RDR);
  c5.setContextOption(MUST_BE_FRESH_S, true);
  //c5.setContextOption(FORWARDING_STRATEGY, BROADCAST);

  c5.setContextOption(INTEREST_LEAVE_CNTX, (ConsumerInterestCallback)bind(&CallbackContainer::processLeavingInterest, &stubs, _1, _2));

  c5.setContextOption(DATA_ENTER_CNTX, (ConsumerDataCallback)bind(&CallbackContainer::processData, &stubs, _1, _2));

  c5.setContextOption(CONTENT_RETRIEVED, (ConsumerContentCallback)bind(&CallbackContainer::processPayload, &stubs, _1, _2, _3));

  c1.asyncConsume(Name());
  c2.asyncConsume(Name());
  c3.asyncConsume(Name());
  c4.asyncConsume(Name());
  c5.asyncConsume(Name());

  Consumer::consumeAll(); // blocks until both contexts finish their work in parallel

  sleep(10);

  return 0;
}

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
  return ndn::examples::main(argc, argv);
}

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
// #include <Consumer-Producer-API/producer-context.hpp>
#include "producer-context.hpp"

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
  onCacheMiss(Producer& p, const Interest& interest)
  {
    std::cout << "REPLY TO " << interest.toUri() << std::endl;
    std::string content = "ECHO";
    Name emptySuffix;

    p.produce(emptySuffix, (uint8_t*)content.c_str(), content.size());
  }

  void
  onNewInterest(Producer& p, const Interest& interest)
  {
    std::cout << "COMES IN " << interest.getName() << std::endl;
  }
};

int
main(int argc, char** argv)
{
  Name sampleName("/a/b/c");

  CallbackContainer stubs;

  Producer p(sampleName);

  //setting callbacks
  p.setContextOption(INTEREST_ENTER_CNTX, (ProducerInterestCallback)bind(&CallbackContainer::onNewInterest, &stubs, _1, _2));

  p.setContextOption(CACHE_MISS, (ProducerInterestCallback)bind(&CallbackContainer::onCacheMiss, &stubs, _1, _2));

  p.attach();

  sleep(10); // because attach() is non-blocking

  return 0;
}

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
  return ndn::examples::main(argc, argv);
}

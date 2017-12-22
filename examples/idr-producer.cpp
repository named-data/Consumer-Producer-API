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
  onPacket(Producer& p, Data& data)
  {
    std::cout << data;

    std::string content = reinterpret_cast<const char*>(data.getContent().value());
    content = content.substr(0, data.getContent().value_size());
    std::cout << content << std::endl;
    std::cout << std::endl;
  }
};


int
main(int argc, char** argv)
{
  Name sampleName("/test");

  CallbackContainer cb;
  Producer* p = new Producer(sampleName);
  p->setContextOption(DATA_LEAVE_CNTX, (ProducerDataCallback)bind(&CallbackContainer::onPacket, &cb, _1, _2));

  p->setContextOption(INFOMAX, true);
  p->setContextOption(INFOMAX_PRIORITY, INFOMAX_SIMPLE_PRIORITY); // generate only lists for the root node
  // p->setContextOption(INFOMAX_PRIORITY, INFOMAX_MERGE_PRIORITY);  // generate lists for all sub-trees
  // p->setContextOption(INFOMAX_UPDATE_INTERVAL, 10000);

  p->attach();

  std::string ac = "a/c-content";
  p->produce(Name("a/c"), (uint8_t*)ac.c_str(), ac.size());

  std::string bd = "a/d-content";
  p->produce(Name("a/d"), (uint8_t*)bd.c_str(), bd.size());

  std::string be = "b/e-content";
  p->produce(Name("b/e"), (uint8_t*)be.c_str(), be.size());

  std::string cf = "c/f-content";
  p->produce(Name("c/f"), (uint8_t*)cf.c_str(), cf.size());

  std::string ag = "a/g-content";
  p->produce(Name("a/g"), (uint8_t*)ag.c_str(), ag.size());

  sleep(300);

  return 0;
}

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
  return ndn::examples::main(argc, argv);
}

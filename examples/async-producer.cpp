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
  void
  onP1(Producer& p, const Interest& interest)
  {
    std::cout << "Producer 1 got " << interest.toUri() << std::endl;
  }

  void
  onP2(Producer& p, const Interest& interest)
  {
    std::cout << "Producer 2 got " << interest.toUri() << std::endl;
  }
};

int
main(int argc, char** argv)
{
  CallbackContainer c;

  Producer p1(Name("/q/w/e"));
  p1.setContextOption(INTEREST_ENTER_CNTX, (ProducerInterestCallback)bind(&CallbackContainer::onP1, &c, _1, _2));
  p1.attach();
  uint8_t* content1 = new uint8_t[100000];
  p1.produce(Name(), content1, 100000);

  Producer p2(Name("/t/y/u"));
  p2.setContextOption(INTEREST_ENTER_CNTX, (ProducerInterestCallback)bind(&CallbackContainer::onP2, &c, _1, _2));
  p2.attach();
  uint8_t* content2 = new uint8_t[100000];
  p2.produce(Name(), content2, 100000);

  Producer p3(Name("/a/s/d"));
  p3.setContextOption(INTEREST_ENTER_CNTX, (ProducerInterestCallback)bind(&CallbackContainer::onP2, &c, _1, _2));
  p3.attach();
  uint8_t* content3 = new uint8_t[100000];
  p3.produce(Name(), content3, 100000);

  Producer p4(Name("/g/h/j"));
  p4.setContextOption(INTEREST_ENTER_CNTX, (ProducerInterestCallback)bind(&CallbackContainer::onP2, &c, _1, _2));
  p4.attach();
  uint8_t* content4 = new uint8_t[100000];
  p4.produce(Name(), content4, 100000);

  Producer p5(Name("/b/n/m"));
  p5.setContextOption(INTEREST_ENTER_CNTX, (ProducerInterestCallback)bind(&CallbackContainer::onP2, &c, _1, _2));
  p5.attach();
  uint8_t* content5 = new uint8_t[100000];
  p5.produce(Name(), content5, 100000);

  std::cout << "sleep" << std::endl;
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

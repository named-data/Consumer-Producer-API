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

#ifndef SIMPLE_DATA_RETRIEVAL_HPP
#define SIMPLE_DATA_RETRIEVAL_HPP

#include "context-default-values.hpp"
#include "context-options.hpp"
#include "data-retrieval-protocol.hpp"
#include "selector-helper.hpp"

namespace ndn {

/*
 * Any communication in NDN network involves Interest/Data exchanges,
 * and Simple Data Retrieval protocol (SDR) is the simplest form of fetching Data from NDN network,
 * corresponding to "one Interest / one Data" pattern.
 * SDR provides no guarantee of Interest or Data delivery.
 * If SDR cannot verify an incoming Data packet, the packet is dropped.
 * SDR can be used by applications that want to directly control Interest transmission
 * and error correction, or have small ADUs that fit in one Data packet.
 */
class SimpleDataRetrieval : public DataRetrievalProtocol
{
public:
  SimpleDataRetrieval(Context* context);

  void
  start();

  void
  stop();

private:
  void
  onData(const Interest& interest, const Data& data);

  void
  onNack(const Interest& interest, const lp::Nack& nack);

  void
  onTimeout(const Interest& interest);

  void
  sendInterest();
};

} // namespace ndn

#endif // SIMPLE_DATA_RETRIEVAL_HPP

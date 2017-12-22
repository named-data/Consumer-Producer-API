/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016 Regents of the University of California.
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

#ifndef INFOMAX_DATA_RETRIEVAL_HPP
#define INFOMAX_DATA_RETRIEVAL_HPP

#include "reliable-data-retrieval.hpp"

namespace ndn {

class InfoMaxDataRetrieval : public DataRetrievalProtocol
{
public:
  InfoMaxDataRetrieval(Context* context);

  ~InfoMaxDataRetrieval();

  void
  start();

  void
  stop();

private:
  void
  processInfoMaxPayload(Consumer&, const uint8_t*, size_t);

  void
  processInfoMaxData(Consumer&, const Data&);

  void
  processLeavingInfoMaxInterest(Consumer&, Interest&);

  void
  processInfoMaxInitPayload(Consumer&, const uint8_t*, size_t);

  void
  processInfoMaxInitData(Consumer&, const Data&);

  void
  processLeavingInfoMaxInitInterest(Consumer&, Interest&);

  void
  convertStringToList(std::string&);

private:
  std::list<shared_ptr<Name>> m_infoMaxList;
  shared_ptr<ReliableDataRetrieval> m_rdr;
  uint64_t m_requestVersion;
  uint64_t m_requestListNum;
  uint64_t m_maxListNum;
  bool m_isInit;
};

} // namespace ndn

#endif // INFOMAX_DATA_RETRIEVAL_HPP
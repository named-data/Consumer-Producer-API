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

#ifndef DATA_RETRIEVAL_PROTOCOL_HPP
#define DATA_RETRIEVAL_PROTOCOL_HPP

#include "context.hpp"
#include <ndn-cxx/util/scheduler.hpp>

namespace ndn {

class Consumer;

class DataRetrievalProtocol
{
public:
  DataRetrievalProtocol(Context* context);

  void
  updateFace();

  bool
  isRunning();

  virtual void
  start() = 0;

  virtual void
  stop() = 0;

protected:
  Context* m_context;
  shared_ptr<ndn::Face> m_face;
  bool m_isRunning;
};

} // namespace ndn

#endif // DATA_RETRIEVAL_PROTOCOL_HPP

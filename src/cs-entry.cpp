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

#include "cs-entry.hpp"

#include <ndn-cxx/util/sha256.hpp>

namespace ndn {
namespace cs {

void
Entry::release()
{
  BOOST_ASSERT(m_layerIterators.empty());

  m_dataPacket.reset();
  m_digest.reset();
  m_nameWithDigest.clear();
}

void
Entry::setData(const Data& data, bool isUnsolicited)
{
  m_isUnsolicited = isUnsolicited;
  m_dataPacket = data.shared_from_this();
  m_digest.reset();

  updateStaleTime();

  m_nameWithDigest = data.getName();
  m_nameWithDigest.append(ndn::name::Component(getDigest()));
}

void
Entry::setData(const Data& data, bool isUnsolicited, const ndn::ConstBufferPtr& digest)
{
  m_dataPacket = data.shared_from_this();
  m_digest = digest;

  updateStaleTime();

  m_nameWithDigest = data.getName();
  m_nameWithDigest.append(ndn::name::Component(getDigest()));
}

void
Entry::updateStaleTime()
{
  m_staleAt = time::steady_clock::now() + m_dataPacket->getFreshnessPeriod();
}

const ndn::ConstBufferPtr&
Entry::getDigest() const
{
  if (!static_cast<bool>(m_digest)) {
    const Block& block = m_dataPacket->wireEncode();
    m_digest = util::Sha256::computeDigest(block.wire(), block.size());
  }

  return m_digest;
}

void
Entry::setIterator(int layer, const Entry::LayerIterators::mapped_type& layerIterator)
{
  m_layerIterators[layer] = layerIterator;
}

void
Entry::removeIterator(int layer)
{
  m_layerIterators.erase(layer);
}

void
Entry::printIterators() const
{
}

} // namespace cs
} // namespace ndn

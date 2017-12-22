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

#ifndef APPLICATION_NACK_HPP
#define APPLICATION_NACK_HPP

#include "common.hpp"
#include "tlv.hpp"

#include <ndn-cxx/util/random.hpp>

namespace ndn {

class ApplicationNack : public Data
{
public:
  class Error : public tlv::Error
  {
  public:
    using tlv::Error::Error;
  };

  enum NackCode { NONE = 0, PRODUCER_DELAY = 1, DATA_NOT_AVAILABLE = 2, INTEREST_NOT_VERIFIED = 3 };

  /**
   * Default constructor.
   */
  ApplicationNack();

  /**
   * Constructs ApplicationNack from a given Interest packet
   */
  ApplicationNack(const Interest& interest, ApplicationNack::NackCode statusCode);

  /**
   * Constructor performing upcasting from Data to ApplicationNack
   */
  explicit ApplicationNack(const Data& data);

  ~ApplicationNack();

  void
  addKeyValuePair(const uint8_t* key, size_t keySize, const uint8_t* value, size_t valueSize);

  void
  addKeyValuePair(std::string key, std::string value);

  std::string
  getValueByKey(std::string key);

  void
  eraseValueByKey(std::string key);

  void
  setCode(ApplicationNack::NackCode statusCode);

  ApplicationNack::NackCode
  getCode();

  void
  setDelay(uint32_t milliseconds);

  uint32_t
  getDelay();

  void
  encode();

  template<encoding::Tag TAG>
  size_t
  wireEncode(EncodingImpl<TAG>& encoder) const;

  void
  decode();

private:
  std::map<std::string, std::string> m_keyValuePairs;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(ApplicationNack);

} //namespace ndn

#endif // APPLICATION_NACK_HPP

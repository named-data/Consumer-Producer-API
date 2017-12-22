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

#include "application-nack.hpp"

namespace ndn {

// BOOST_CONCEPT_ASSERT((WireEncodable<ApplicationNack>));
BOOST_CONCEPT_ASSERT((WireEncodableWithEncodingBuffer<ApplicationNack>));
// BOOST_CONCEPT_ASSERT((WireDecodable<ApplicationNack>));
static_assert(std::is_base_of<tlv::Error, ApplicationNack::Error>::value, "ApplicationNack::Error must inherit from tlv::Error");

// NACK Headers
const std::string STATUS_CODE_H = "Status-code";
const std::string RETRY_AFTER_H = "Retry-after";

ApplicationNack::ApplicationNack()
{
  setContentType(tlv::ContentType_Nack);
  setCode(ApplicationNack::NONE);
}

ApplicationNack::ApplicationNack(const Interest& interest, ApplicationNack::NackCode statusCode)
{
  Name name = interest.getName();
  name.append(Name("nack"));
  name.appendNumber(ndn::random::generateSecureWord64());
  setName(name);
  setContentType(tlv::ContentType_Nack);
  setCode(statusCode);
}

ApplicationNack::ApplicationNack(const Data& data)
  : Data(data)
{
  setContentType(tlv::ContentType_Nack);
  decode();
}

ApplicationNack::~ApplicationNack()
{
}

void
ApplicationNack::addKeyValuePair(const uint8_t* key, size_t keySize, const uint8_t* value, size_t valueSize)
{
  std::string keyS(reinterpret_cast<const char*>(key), keySize);
  std::string valueS(reinterpret_cast<const char*>(value), valueSize);
  addKeyValuePair(keyS, valueS);
}

void
ApplicationNack::addKeyValuePair(std::string key, std::string value)
{
  m_keyValuePairs[key] = value;
}

std::string
ApplicationNack::getValueByKey(std::string key)
{
  std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(key);

  if (it == m_keyValuePairs.end()) {
    return "";
  }
  else {
    return it->second;
  }
}

void
ApplicationNack::eraseValueByKey(std::string key)
{
  m_keyValuePairs.erase(m_keyValuePairs.find(key));
}

void
ApplicationNack::setCode(ApplicationNack::NackCode statusCode)
{
  std::stringstream ss;
  ss << statusCode;
  std::string value = ss.str();
  addKeyValuePair(STATUS_CODE_H, value);
}

ApplicationNack::NackCode
ApplicationNack::getCode()
{
  std::string value = getValueByKey(STATUS_CODE_H);

  if (value != "") {
    try {
      return (ApplicationNack::NackCode)atoi(value.c_str());
    }
    catch (std::exception e) {
      return ApplicationNack::NONE;
    }
  }
  else {
    return ApplicationNack::NONE;
  }
}

void
ApplicationNack::setDelay(uint32_t milliseconds)
{
  std::stringstream ss;
  ss << milliseconds;
  std::string value = ss.str();
  addKeyValuePair(RETRY_AFTER_H, value);
}

uint32_t
ApplicationNack::getDelay()
{
  std::string value = getValueByKey(RETRY_AFTER_H);
  return atoi(value.c_str());
}

template<encoding::Tag TAG>
size_t
ApplicationNack::wireEncode(EncodingImpl<TAG>& encoder) const
{
  // Nack ::= CONTENT-TLV TLV-LENGTH
  //            KeyValuePair*

  size_t totalLength = 0;

  for (std::map<std::string, std::string>::const_reverse_iterator it = m_keyValuePairs.rbegin(); it != m_keyValuePairs.rend(); ++it) {
    std::string keyValue = it->first + "=" + it->second;
    totalLength += encoder.prependByteArray(reinterpret_cast<const uint8_t*>(keyValue.c_str()), keyValue.size());
    totalLength += encoder.prependVarNumber(keyValue.size());
    totalLength += encoder.prependVarNumber(tlv::KeyValuePair);
  }

  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(ApplicationNack);

void
ApplicationNack::encode()
{
  EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  setContentType(tlv::ContentType_Nack);
  setContent(const_cast<uint8_t*>(buffer.buf()), buffer.size());
}

void
ApplicationNack::decode()
{
  Block content = getContent();
  content.parse();

  // Nack ::= CONTENT-TLV TLV-LENGTH
  //                KeyValuePair*

  for (const auto& val : content.elements()) {
    if (val.type() == tlv::KeyValuePair) {
      std::string str(reinterpret_cast<const char*>(val.value()), val.value_size());

      size_t index = str.find_first_of('=');
      if (index == std::string::npos || index == 0 || (index == str.size() - 1))
        continue;

      std::string key = str.substr(0, index);
      std::string value = str.substr(index + 1, str.size() - index - 1);
      addKeyValuePair(key, value);
    }
  }
}

} // namespace ndn

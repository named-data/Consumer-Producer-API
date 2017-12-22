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

#include "manifest.hpp"

namespace ndn {

// BOOST_CONCEPT_ASSERT((WireEncodable<Manifest>));
BOOST_CONCEPT_ASSERT((WireEncodableWithEncodingBuffer<Manifest>));
// BOOST_CONCEPT_ASSERT((WireDecodable<Manifest>));
static_assert(std::is_base_of<tlv::Error, Manifest::Error>::value, "Manifest::Error must inherit from tlv::Error");

Manifest::Manifest()
{
  setContentType(tlv::ContentType_Manifest);
}

Manifest::Manifest(const Name& name)
  : Data(name)
{
  setContentType(tlv::ContentType_Manifest);
}

Manifest::Manifest(const Data& data)
  : Data(data)
{
  setContentType(tlv::ContentType_Manifest);
  decode();
}

Manifest::~Manifest()
{
}

void
Manifest::addKeyValuePair(const uint8_t* key, size_t keySize, const uint8_t* value, size_t valueSize)
{
  std::string keyS(reinterpret_cast<const char*>(key), keySize);
  std::string valueS(reinterpret_cast<const char*>(value), valueSize);
  addKeyValuePair(keyS, valueS);
}

void
Manifest::addKeyValuePair(std::string key, std::string value)
{
  m_keyValuePairs[key] = value;
}

std::string
Manifest::getValueByKey(std::string key)
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
Manifest::eraseValueByKey(std::string key)
{
  m_keyValuePairs.erase(m_keyValuePairs.find(key));
}

void
Manifest::addNameToCatalogue(const Name& name)
{
  m_catalogueNames.push_back(name);
}

void
Manifest::addNameToCatalogue(const Name& name, const Block& digest)
{
  Name fullName(name);
  fullName.append(ndn::name::Component::fromImplicitSha256Digest(digest.value(), digest.value_size()));
  m_catalogueNames.push_back(fullName);
}

void
Manifest::addNameToCatalogue(const Name& name, const ndn::ConstBufferPtr& digest)
{
  Name fullName(name);
  fullName.append(ndn::name::Component::fromImplicitSha256Digest(digest));
  m_catalogueNames.push_back(fullName);
}

template<encoding::Tag TAG>
size_t
Manifest::wireEncode(EncodingImpl<TAG>& encoder) const
{
  // Manifest ::= CONTENT-TLV TLV-LENGTH
  //                Catalogue?
  //                  Name*
  //                KeyValuePair*

  size_t totalLength = 0;
  size_t catalogueLength = 0;

  for (std::map<std::string, std::string>::const_reverse_iterator it = m_keyValuePairs.rbegin(); it != m_keyValuePairs.rend(); ++it) {
    std::string keyValue = it->first + "=" + it->second;
    totalLength += encoder.prependByteArray(reinterpret_cast<const uint8_t*>(keyValue.c_str()), keyValue.size());
    totalLength += encoder.prependVarNumber(keyValue.size());
    totalLength += encoder.prependVarNumber(tlv::KeyValuePair);
  }

  for (std::list<Name>::const_reverse_iterator it = m_catalogueNames.rbegin(); it != m_catalogueNames.rend(); ++it) {
    size_t blockSize = encoder.prependBlock(it->wireEncode());
    totalLength += blockSize;
    catalogueLength += blockSize;
  }

  if (catalogueLength > 0) {
    totalLength += encoder.prependVarNumber(catalogueLength);
    totalLength += encoder.prependVarNumber(tlv::ManifestCatalogue);
  }

  //totalLength += encoder.prependVarNumber(totalLength);
  //totalLength += encoder.prependVarNumber(tlv::Content);
  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(Manifest);

void
Manifest::encode()
{
  EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  setContentType(tlv::ContentType_Manifest);
  setContent(const_cast<uint8_t*>(buffer.buf()), buffer.size());
}

void
Manifest::decode()
{
  Block content = getContent();
  content.parse();

  // Manifest ::= CONTENT-TLV TLV-LENGTH
  //                Catalogue?
  //                  Name*
  //                KeyValuePair*

  for (Block::element_const_iterator val = content.elements_begin(); val != content.elements_end(); ++val) {
    if (val->type() == tlv::ManifestCatalogue) {
      val->parse();
      for (Block::element_const_iterator catalogueNameElem = val->elements_begin(); catalogueNameElem != val->elements_end();
           ++catalogueNameElem) {
        if (catalogueNameElem->type() == tlv::Name) {
          Name name(*catalogueNameElem);
          m_catalogueNames.push_back(name);
        }
      }
    }
    else if (val->type() == tlv::KeyValuePair) {
      std::string str((char*)val->value(), val->value_size());

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

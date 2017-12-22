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

#ifndef MANIFEST_HPP
#define MANIFEST_HPP

#include "common.hpp"
#include "context-default-values.hpp"
#include "context-options.hpp"
#include "context.hpp"
#include "tlv.hpp"

#include <ndn-cxx/encoding/tlv.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/sha256.hpp>

namespace ndn {

class Manifest : public Data
{
public:
  class Error : public tlv::Error
  {
  public:
    using tlv::Error::Error;
  };

  /**
   * Default constructor.
   */
  Manifest();

  explicit Manifest(const Name& name);

  explicit Manifest(const Data& data);

  /**
   * The virtual destructor.
   */
  virtual ~Manifest();

  inline void
  wireDecode(const Block& wire);

  void
  addKeyValuePair(const uint8_t* key, size_t keySize, const uint8_t* value, size_t valueSize);

  void
  addKeyValuePair(std::string key, std::string value);

  std::string
  getValueByKey(std::string key);

  void
  eraseValueByKey(std::string key);

  /**
   * Begin iterator (const).
   */
  std::list<Name>::const_iterator
  catalogueBegin() const
  {
    return m_catalogueNames.begin();
  }

  /**
   * End iterator (const).
   */
  std::list<Name>::const_iterator
  catalogueEnd() const
  {
    return m_catalogueNames.end();
  }

  /**
   * Adds full name (with digest) to the manifest.
   * @param name Name must contain a digest in its last component.
   */
  void
  addNameToCatalogue(const Name& name);

  /**
   * Concatenates the name prefix with the digest and adds the resulting full name to the manifest.
   * @param name The name prefix without digest component.
   * @param digest The tlv block containing digest (data.getSignature().getValue())
   */
  void
  addNameToCatalogue(const Name& name, const Block& digest);

  /**
   * Concatenates the name prefix with the digest and adds the resulting full name to the manifest.
   * @param name The name prefix without digest component.
   * @param digest The buffer containing digest
   */
  void
  addNameToCatalogue(const Name& name, const ndn::ConstBufferPtr& digest);

  void
  eraseNameFromCatalogue(const std::vector<Name>::iterator it);

  /**
   * encode manifest into content (Data packet)
   */
  void
  encode();

  void
  decode();

  template<encoding::Tag TAG>
  size_t
  wireEncode(EncodingImpl<TAG>& encoder) const;

private:
  std::list<Name> m_catalogueNames;
  std::map<std::string, std::string> m_keyValuePairs;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(Manifest);

} //namespace ndn

#endif // MANIFEST_HPP

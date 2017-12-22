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

#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "common.hpp"

#include "application-nack.hpp"
#include "context-default-values.hpp"
#include "context-options.hpp"
#include "face-helper.hpp"
#include "infomax-tree-node.hpp"
#include "manifest.hpp"
#include "tlv.hpp"

#include <unordered_map>

namespace ndn {

class Manifest;
class Consumer;
class Producer;

typedef function<void(Consumer&, Interest&)> ConsumerInterestCallback;
typedef function<void(Consumer&, const uint8_t*, size_t)> ConsumerContentCallback;
typedef function<void(Consumer&, const Data&)> ConsumerDataCallback;
typedef function<bool(Consumer&, const Data&)> ConsumerDataVerificationCallback;
typedef function<void(Consumer&, const ApplicationNack&)> ConsumerNackCallback;
typedef function<void(Consumer&, const Manifest&)> ConsumerManifestCallback;

typedef function<void(Producer&, Data&)> ProducerDataCallback;
typedef function<void(Producer&, const Interest&)> ProducerInterestCallback;

class Context
{
public:
  /*
   * Context option setters
   */
  virtual int
  setContextOption(int optionName, int optionValue) = 0;

  virtual int
  setContextOption(int optionName, size_t optionValue) = 0;

  virtual int
  setContextOption(int optionName, bool optionValue) = 0;

  virtual int
  setContextOption(int optionName, Name optionValue) = 0;

  virtual int
  setContextOption(int optionName, ProducerDataCallback optionValue) = 0;

  virtual int
  setContextOption(int optionName, ProducerInterestCallback optionValue) = 0;

  virtual int
  setContextOption(int optionName, ConsumerDataVerificationCallback optionValue) = 0;

  virtual int
  setContextOption(int optionName, ConsumerDataCallback optionValue) = 0;

  virtual int
  setContextOption(int optionName, ConsumerInterestCallback optionValue) = 0;

  virtual int
  setContextOption(int optionName, ConsumerContentCallback optionValue) = 0;

  virtual int
  setContextOption(int optionName, ConsumerNackCallback optionValue) = 0;

  virtual int
  setContextOption(int optionName, ConsumerManifestCallback optionValue) = 0;

  virtual int
  setContextOption(int optionName, KeyLocator optionValue) = 0;

  virtual int
  setContextOption(int optionName, Exclude optionValue) = 0;

  /*
   * Context option getters
   */
  virtual int
  getContextOption(int optionName, int& optionValue) = 0;

  virtual int
  getContextOption(int optionName, size_t& optionValue) = 0;

  virtual int
  getContextOption(int optionName, bool& optionValue) = 0;

  virtual int
  getContextOption(int optionName, Name& optionValue) = 0;

  virtual int
  getContextOption(int optionName, ProducerDataCallback& optionValue) = 0;

  virtual int
  getContextOption(int optionName, ProducerInterestCallback& optionValue) = 0;

  virtual int
  getContextOption(int optionName, ConsumerDataVerificationCallback& optionValue) = 0;

  virtual int
  getContextOption(int optionName, ConsumerDataCallback& optionValue) = 0;

  virtual int
  getContextOption(int optionName, ConsumerInterestCallback& optionValue) = 0;

  virtual int
  getContextOption(int optionName, ConsumerContentCallback& optionValue) = 0;

  virtual int
  getContextOption(int optionName, ConsumerNackCallback& optionValue) = 0;

  virtual int
  getContextOption(int optionName, ConsumerManifestCallback& optionValue) = 0;

  virtual int
  getContextOption(int optionName, KeyLocator& optionValue) = 0;

  virtual int
  getContextOption(int optionName, Exclude& optionValue) = 0;

  virtual int
  getContextOption(int optionName, shared_ptr<Face>& optionValue) = 0;

  virtual int
  getContextOption(int optionName, TreeNode& optionValue) = 0;

protected:
  ~Context(){};
};

} // namespace ndn

#endif // CONTEXT_HPP

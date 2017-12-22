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

#ifndef CONTEXT_DEFAULT_VALUES_HPP
#define CONTEXT_DEFAULT_VALUES_HPP

#include <ndn-cxx/encoding/tlv.hpp>
#include <ndn-cxx/name.hpp>

// this file contains various default values
// numbers here are not unique

#define EMPTY_CALLBACK 0

// protocols
#define SDR 0
#define UDR 1
#define RDR 2
#define IDR 3

// forwarding strategies
const ndn::Name BEST_ROUTE("ndn:/localhost/nfd/strategy/best-route");
const ndn::Name BROADCAST("ndn:/localhost/nfd/strategy/broadcast");
const ndn::Name CLIENT_CONTROL("ndn:/localhost/nfd/strategy/client-control");
//const ndn::Name NCC("ndn:/localhost/nfd/strategy/ncc");

// default values
#define DEFAULT_INTEREST_LIFETIME_API 200 // milliseconds
#define DEFAULT_DATA_FRESHNESS 100000 // milliseconds ~= 100 seconds
#define DEFAULT_DATA_PACKET_SIZE 2048 // bytes
#define DEFAULT_INTEREST_SCOPE 2
#define DEFAULT_MIN_SUFFIX_COMP -1
#define DEFAULT_MAX_SUFFIX_COMP -1
#define DEFAULT_PRODUCER_RCV_BUFFER_SIZE 1000 // of Interests
#define DEFAULT_PRODUCER_SND_BUFFER_SIZE 1000 // of Data
#define DEFAULT_KEY_LOCATOR_SIZE 256          // of bytes
#define DEFAULT_SAFETY_OFFSET 10              // of bytes
#define DEFAULT_MIN_WINDOW_SIZE 4             // of Interests
#define DEFAULT_MAX_WINDOW_SIZE 64            // of Interests
#define DEFAULT_DIGEST_SIZE 32                // of bytes
#define DEFAULT_FAST_RETX_CONDITION 3         // of out-of-order segments

// maximum allowed values
#define CONSUMER_MIN_RETRANSMISSIONS 0
#define CONSUMER_MAX_RETRANSMISSIONS 32
#define DEFAULT_MAX_EXCLUDED_DIGESTS 5
#define MAX_DATA_PACKET_SIZE 8096

// set/getcontextoption values
#define OPTION_FOUND 0
#define OPTION_NOT_FOUND 1
#define OPTION_VALUE_SET 2
#define OPTION_VALUE_NOT_SET 3
#define OPTION_DEFAULT_VALUE 666 // some rare number

// misc. values
#define PRODUCER_OPERATION_FAILED 10
#define CONSUMER_READY 0
#define CONSUMER_BUSY 1

#define REGISTRATION_NOT_ATTEMPTED 0
#define REGISTRATION_SUCCESS 1
#define REGISTRATION_FAILURE 2
#define REGISTRATION_IN_PROGRESS 3

#define LEFTMOST_CHILD 0
#define RIGHTMOST_CHILD 1

#define SHA_256 1
#define RSA_256 2

// Negative acknowledgement related constants
#define NACK_DATA_TYPE tlv::ContentType_Nack

#define NACK_DELAY 1
#define NACK_INTEREST_NOT_VERIFIED 2

// Manifest related constants
#define MANIFEST_DATA_TYPE tlv::ContentType_Manifest

#define FULL_NAME_ENUMERATION 0
#define DIGEST_ENUMERATION 1

#define CONTENT_DATA_TYPE tlv::ContentType_Blob

// InfoMax parameter
#define INFOMAX_DEFAULT_LIST_SIZE 10
#define INFOMAX_INTEREST_TAG "InfoMax"
#define INFOMAX_META_INTEREST_TAG "MetaInfo"
#define INFOMAX_DEFAULT_UPDATE_INTERVAL 5000 // 5 seconds

// InfoMax prioritizer
#define INFOMAX_NONE 0
#define INFOMAX_SIMPLE_PRIORITY 1
#define INFOMAX_MERGE_PRIORITY 2


#endif

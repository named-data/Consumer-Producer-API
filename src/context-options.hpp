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

#ifndef CONTEXT_OPTIONS_HPP
#define CONTEXT_OPTIONS_HPP

// This file contains recognized values for the first parameter of get/setContextOption
// every number is unique

#define RCV_BUF_SIZE            1   // int
#define SND_BUF_SIZE            2   // int
#define PREFIX                  3   // Name
#define SUFFIX                  4   // Name
#define REMOTE_REPO_PREFIX      5   // Name
#define LOCAL_REPO              6   // bool
#define INTEREST_RETX           7   // int
#define DATA_PKT_SIZE           8   // int
#define INTEREST_LIFETIME       9   // int
#define FORWARDING_STRATEGY     10  // int
#define DATA_FRESHNESS          11  // int
#define REGISTRATION_STATUS     12  // int
#define KEY_LOCATOR             13  // KeyLocator
#define SIGNATURE_TYPE          14  // int
#define MIN_WINDOW_SIZE         15  // int
#define MAX_WINDOW_SIZE         16  // int
#define CURRENT_WINDOW_SIZE     17  // int
#define MAX_EXCLUDED_DIGESTS    18  // int
#define ASYNC_MODE              19  // bool
#define FAST_SIGNING            20  // bool
#define FACE                    21  // Face
#define RUNNING                 22  // bool
#define INFOMAX                 23  // bool
#define INFOMAX_ROOT            24  // TreeNode
#define INFOMAX_PRIORITY        25  // int
#define INFOMAX_UPDATE_INTERVAL 26  // int (milliseconds) 

// selectors
#define MIN_SUFFIX_COMP_S   101 // int
#define MAX_SUFFIX_COMP_S   102 // int
#define EXCLUDE_S           103 // Exclude
#define MUST_BE_FRESH_S     104 // bool
#define LEFTMOST_CHILD_S    105 // int
#define RIGHTMOST_CHILD_S   106 // int
#define KEYLOCATOR_S        107 // KeyLocator

// consumer context events
#define INTEREST_LEAVE_CNTX       201 // InterestCallback
#define INTEREST_RETRANSMIT       202 // InterestCallback
#define INTEREST_EXPIRED          203 // ConstInterestCallback
#define INTEREST_SATISFIED        204 // ConstInterestCallback

#define DATA_ENTER_CNTX           211 // DataCallback
#define NACK_ENTER_CNTX           212 // ConstNackCallback
#define MANIFEST_ENTER_CNTX       213 // ConstManifestCallback
#define DATA_TO_VERIFY            214 // DataVerificationCallback
#define CONTENT_RETRIEVED         215 // ContentCallback

// producer context events
#define INTEREST_ENTER_CNTX       301   
#define INTEREST_DROP_RCV_BUF     302
#define INTEREST_PASS_RCV_BUF     303
#define CACHE_HIT                 306
#define CACHE_MISS                308
#define NEW_DATA_SEGMENT          309 // DataCallback
#define DATA_TO_SECURE            313 // DataCallback
#define DATA_IN_SND_BUF           310 // DataCallback
#define DATA_LEAVE_CNTX           311 // ConstDataCallback
#define DATA_EVICT_SND_BUF        312 // ConstDataCallback

#endif

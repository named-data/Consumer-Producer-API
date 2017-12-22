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

/** \file
 *  \brief import common constructs for Consumer/Producer API library internal use
 *  \warning This file is implementation detail of Consumer/Producer API library.
 *           Aliases imported in this file SHOULD NOT be used outside of Consumer/Producer API.
 */

#ifndef CONSUMERPRODUCER_COMMON_HPP
#define CONSUMERPRODUCER_COMMON_HPP

// require C++11
#if __cplusplus < 201103L && !defined(__GXX_EXPERIMENTAL_CXX0X__)
#error "ndn-cxx applications must be compiled using the C++11 standard"
#endif

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unistd.h>
#include <utility>

#include <ndn-cxx/common.hpp>
#include <ndn-cxx/data.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>

/*#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/assert.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/tuple/tuple.hpp>
*/
#if defined(__GNUC__) || defined(__clang__)
#define DEPRECATED(func) func __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED(func) __declspec(deprecated) func
#else
#pragma message("DEPRECATED not implemented")
#define DEPRECATED(func) func
#endif

namespace ndn {

using std::bad_weak_ptr;
using std::enable_shared_from_this;
using std::make_shared;
using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;

using std::const_pointer_cast;
using std::dynamic_pointer_cast;
using std::static_pointer_cast;

using std::bind;
using std::cref;
using std::function;
using std::ref;

} // namespace ndn

#endif // CONSUMERPRODUCER_COMMON_HPP

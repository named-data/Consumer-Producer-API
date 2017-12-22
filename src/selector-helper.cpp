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


#include "selector-helper.hpp"

namespace ndn {

void
SelectorHelper::applySelectors(Interest& interest, Context* context)
{
  int minSuffix = -1;
  context->getContextOption(MIN_SUFFIX_COMP_S, minSuffix);

  if (minSuffix >= 0) {
    interest.setMinSuffixComponents(minSuffix);
  }

  int maxSuffix = -1;
  context->getContextOption(MAX_SUFFIX_COMP_S, maxSuffix);

  if (maxSuffix >= 0) {
    interest.setMaxSuffixComponents(maxSuffix);
  }

  Exclude exclusion;
  context->getContextOption(EXCLUDE_S, exclusion);

  if (!exclusion.empty()) {
    interest.setExclude(exclusion);
  }

  bool mustBeFresh = false;
  context->getContextOption(MUST_BE_FRESH_S, mustBeFresh);

  if (mustBeFresh) {
    interest.setMustBeFresh(mustBeFresh);
  }

  int child = -10;
  context->getContextOption(RIGHTMOST_CHILD_S, child);

  if (child != -10) {
    interest.setChildSelector(child);
  }

  KeyLocator keyLocator;
  context->getContextOption(KEYLOCATOR_S, keyLocator);

  if (!keyLocator.empty()) {
    interest.setPublisherPublicKeyLocator(keyLocator);
  }
}

} //namespace ndn

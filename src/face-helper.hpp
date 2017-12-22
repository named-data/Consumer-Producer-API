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

#ifndef FACE_HELPER_HPP
#define FACE_HELPER_HPP

#include "common.hpp"

namespace ndn {

class FaceHelper
{
public:
  static shared_ptr<Face>
  getFace();

private:
  FaceHelper(){};
  FaceHelper(const FaceHelper& helper){}; // copy constructor is private
  FaceHelper&
  operator=(const FaceHelper& helper)
  {
    return *this;
  }; // assignment operator is private
  static shared_ptr<Face> m_face;
};

} // namespace ndn

#endif // FACE_HELPER_HPP

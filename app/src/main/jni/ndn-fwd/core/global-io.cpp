/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2015,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "global-io.hpp"
#include <boost/thread/tss.hpp>

namespace nfd {

namespace scheduler {
// defined in scheduler.cpp
void
resetGlobalScheduler();
} // namespace scheduler


static boost::thread_specific_ptr<boost::asio::io_service> g_ioService;

boost::asio::io_service&
getGlobalIoService()
{
  if (g_ioService.get() == nullptr) {
    g_ioService.reset(new boost::asio::io_service());
  }
  return *g_ioService;
}

void
resetGlobalIoService()
{
  scheduler::resetGlobalScheduler();
  g_ioService.reset();
}

} // namespace nfd

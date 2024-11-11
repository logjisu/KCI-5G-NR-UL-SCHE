/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#pragma once

#include "nr-mac-scheduler-ns3.h"

namespace ns3 {

class NrMacSchedulerUeInfoAG : public NrMacSchedulerUeInfo
{
public:
  NrMacSchedulerUeInfoAG (uint16_t rnti, BeamConfId beamConfId, const GetRbPerRbgFn &fn)
   : NrMacSchedulerUeInfo (rnti, beamConfId, fn)
  {
  }

  static bool CompareUeWeightsDl (const NrMacSchedulerNs3::UePtrAndBufferReq &lue,
                                  const NrMacSchedulerNs3::UePtrAndBufferReq & rue)
  {
    auto luePtr = dynamic_cast<NrMacSchedulerUeInfoAG*>(lue.first.get());
    auto ruePtr = dynamic_cast<NrMacSchedulerUeInfoAG*>(rue.first.get());

    // Age가 큰 UE를 더 높은 우선순위로 설정
    return luePtr->m_age > ruePtr->m_age;
  }

  static bool CompareUeWeightsUl (const NrMacSchedulerNs3::UePtrAndBufferReq &lue,
                                  const NrMacSchedulerNs3::UePtrAndBufferReq & rue)
  {
    auto luePtr = dynamic_cast<NrMacSchedulerUeInfoAG*>(lue.first.get());
    auto ruePtr = dynamic_cast<NrMacSchedulerUeInfoAG*>(rue.first.get());
    // Age가 큰 UE를 더 높은 우선순위로 설정
    return luePtr->m_age > ruePtr->m_age;
  }

  uint64_t m_age {0};
};

} // namespace ns3

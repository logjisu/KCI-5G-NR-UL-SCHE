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
#include "nr-mac-scheduler-ofdma-ag.h"
#include "nr-mac-scheduler-ue-info-ag.h"
#include <ns3/log.h>


namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerOfdmaAG");


TypeId NrMacSchedulerOfdmaAG::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrMacSchedulerOfdmaAG")
    .SetParent<NrMacSchedulerOfdma> ()
    .AddConstructor<NrMacSchedulerOfdmaAG> ();
  return tid;
}

NrMacSchedulerOfdmaAG::NrMacSchedulerOfdmaAG () : NrMacSchedulerOfdma (){
  
}

std::shared_ptr<NrMacSchedulerUeInfo>
NrMacSchedulerOfdmaAG::CreateUeRepresentation (const NrMacCschedSapProvider::CschedUeConfigReqParameters& params) const
{
  NS_LOG_FUNCTION (this);
  return std::make_shared<NrMacSchedulerUeInfoAG> (params.m_rnti, params.m_beamConfId,
                                                  std::bind(&NrMacSchedulerOfdmaAG::GetNumRbPerRbg, this));
}

void NrMacSchedulerOfdmaAG::AssignedDlResources (const UePtrAndBufferReq &ue,
                                            [[maybe_unused]] const FTResources &assigned,
                                            [[maybe_unused]] const FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  GetFirst GetUe;
  GetUe (ue)->UpdateDlMetric (m_dlAmc);
}

void
NrMacSchedulerOfdmaAG::AssignedUlResources (const UePtrAndBufferReq &ue,
                                            [[maybe_unused]] const FTResources &assigned,
                                            [[maybe_unused]] const FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  GetFirst GetUe;
  GetUe (ue)->UpdateUlMetric (m_ulAmc);
}

std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                   const NrMacSchedulerNs3::UePtrAndBufferReq &rhs)>
NrMacSchedulerOfdmaAG::GetUeCompareDlFn () const
{
  NS_LOG_FUNCTION (this);
  // Sort UEs based on Age (descending order for priority)
  return NrMacSchedulerUeInfoAG::CompareUeWeightsDl;
}

std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                   const NrMacSchedulerNs3::UePtrAndBufferReq &rhs)>
NrMacSchedulerOfdmaAG::GetUeCompareUlFn () const
{
  return [this](const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                const NrMacSchedulerNs3::UePtrAndBufferReq &rhs) {
    uint16_t lueRnti = lhs.first->m_rnti;
    uint16_t rueRnti = rhs.first->m_rnti;

    uint64_t left_age = NrMacSchedulerNs3::GetAge(lueRnti);
    uint64_t right_age = NrMacSchedulerNs3::GetAge(rueRnti);

    NS_LOG_INFO("비교 1(좌) UE: " << lueRnti << "\t Age: " << left_age << "\t | \t비교 2(우) UE: " << rueRnti << "\t Age: " << right_age);
    return left_age > right_age;
  };
}

} // namespace ns3
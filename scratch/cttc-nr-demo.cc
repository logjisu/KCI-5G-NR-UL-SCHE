//Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil;
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
/**
 * \brief 편안하고 쉬운 5G NR 코드(튜토리얼, 데모방식)
 *
 * 이 코드는 TR 38.900의 3GPP 채널 모델을 사용하여 시뮬레이션을 설정하는 방법을 설명합니다.
 * 
 * 3GPP의 TR 38.900은 주로 5G와 그 이상의 무선 통신 기술에 필요한 다양한 주파수 대역에서의 채널 특성을 연구하고 분석한다.
 * 특히, 이 보고서는 밀리미터파(mmWave)와 같은 고주파 대역에서의 전파 전파 특성, 채널 손실, 간섭 및 다중 경로 효과 등에 대한 정보를 제공한다.
 * 이를 통해 5G 네트워크의 설계와 성능 최적화에 중요한 데이터를 제공한다.
 * 요약하자면, TR 38.900은 5G 및 그 이상 기술의 무선 채널 모델링에 필수적인 정보를 제공하는 기술 보고서입니다. 
 * 
 * 간단한 그리드 토폴로지로 구성되어 있으며, 여기서 gNB 및 UE의 수를 설정할 수 있다.
 * 가능한 매개 변수를 살펴보고 명령줄을 통해 무엇을 구성할 수 있는지 보아라.
 *
 * 기본 구성에서는 2개의 서로 다른 서브밴드 뉴몰로지(또는 대역폭 부분)을 통과하는 2개의 흐름을 생성한다.
 * 이를 위해 구체적으로 각각 하나의 CC가 있는 2개의 대역폭이 생성되며, 각 CC에는 하나의 대역폭 부분이 포함된다.
 *
 * 이 예제는 1 ~ 2개의 흐름의 종단 간 결과를 화면에 인쇄하고 파일에 기록한다.
 *
 * \code{.unparsed}
$ ./ns3 run "cttc-nr-demo --PrintHelp"
    \endcode
 *
 */

/*
 * 추가 구역
 * 종종 전체 모듈의 헤더를 포함해야 한다.
 * "-module.h"와 함께 필요한 모듈의 이름을 포함하여 이 작업을 수행한다.
 */

#include "ns3/core-module.h"
#include "ns3/config-store-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/buildings-module.h"
#include "ns3/nr-module.h"
#include "ns3/antenna-module.h"
#include "ns3/config-store.h"
#include "ns3/log.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/nr-helper.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/nr-mac-scheduler-ofdma.h"
#include "ns3/ideal-beamforming-algorithm.h"


//항상 namespace ns3를 사용한다. 모든 NR 클래스는 namespace 안에 있다.
using namespace ns3;


//이 줄을 사용하면 "CttcNrDemo" 구성 요소를 활성화하여 파일의 로그를 볼 수 있다.
NS_LOG_COMPONENT_DEFINE ("CttcNrDemo");


int main (int argc, char *argv[]){
  /*
   * 커맨드 라인에서 입력으로 수락할 매개 변수를 나타내는 변수입니다.
   * 각 변수는 기본값으로 초기화되며, 커맨드 라인 인수를 구문 분석할 때 아래에서 재정의될 수 있습니다.
   */
  // 네트워크 토폴리지 시나리오 파라미터(이 스크립트 內에서 사용될 것이다.):
  uint16_t gNbNum = 1;
  uint16_t ueNumPergNb = 4;
  bool logging = false;
  bool doubleOperationalBand = true;


  // 네트워크 트래픽 파라미터(이 스크립트 內에서 사용될 것이다.):
  uint32_t udpPacketSizeULL = 100;  // 100 bytes
  uint32_t udpPacketSizeBe = 1252;  // 1252 bytes
  uint32_t lambdaULL = 10000;
  uint32_t lambdaBe = 10000;


  /*
  * 시뮬레이션 파라미터. 
  * 제발!!!!! '실수(double)'형을 사용하여 초를 사용하지 마세요!!!!!
  * 휴대성 이슈를 피하기 위해 '정수(int)'형을 사용하는 NS-3 시간 값을 사용합니다.
  */
  Time simTime = MilliSeconds (1000);
  Time udpAppStartTime = MilliSeconds (400);


  /*
  * 5G NR의 파라미터이다.
  * 커맨드 라인에서 입력을 받은 다음 NR 모듈 내부로 전달합니다.
  */
  uint16_t numerologyBwp1 = 4;
  double centralFrequencyBand1 = 28e9;
  double bandwidthBand1 = 100e6;
  uint16_t numerologyBwp2 = 2;
  double centralFrequencyBand2 = 28.2e9;
  double bandwidthBand2 = 100e6;
  double totalTxPower = 4;


  // 출력 파일을 저장할 위치이다.
  std::string simTag = "default";
  std::string outputDir = "./";


  /*
   * 아래 코드부터는 입력으로 허용할 수 있는 모든 입력 매개 변수와 설명 및 저장 변수의
   * ns3::CommandLine 클래스를 지시합니다.
   */
  CommandLine cmd;
  cmd.AddValue ("gNbNum", "The number of gNbs in multiple-ue topology", gNbNum);
  cmd.AddValue ("ueNumPergNb", "The number of UE per gNb in multiple-ue topology", ueNumPergNb);
  cmd.AddValue ("logging", "Enable logging", logging);
  cmd.AddValue ("doubleOperationalBand", "If true, simulate two operational bands with one CC for each band,"
                "and each CC will have 1 BWP that spans the entire CC.", doubleOperationalBand);
  cmd.AddValue ("packetSizeUll", "packet size in bytes to be used by ultra low latency traffic", udpPacketSizeULL);
  cmd.AddValue ("packetSizeBe", "packet size in bytes to be used by best effort traffic", udpPacketSizeBe);
  cmd.AddValue ("lambdaUll", "Number of UDP packets in one second for ultra low latency traffic", lambdaULL);
  cmd.AddValue ("lambdaBe", "Number of UDP packets in one second for best effor traffic", lambdaBe);
  cmd.AddValue ("simTime", "Simulation time", simTime);
  cmd.AddValue ("numerologyBwp1", "The numerology to be used in bandwidth part 1", numerologyBwp1);
  cmd.AddValue ("centralFrequencyBand1", "The system frequency to be used in band 1", centralFrequencyBand1);
  cmd.AddValue ("bandwidthBand1", "The system bandwidth to be used in band 1", bandwidthBand1);
  cmd.AddValue ("numerologyBwp2", "The numerology to be used in bandwidth part 2", numerologyBwp2);
  cmd.AddValue ("centralFrequencyBand2", "The system frequency to be used in band 2", centralFrequencyBand2);
  cmd.AddValue ("bandwidthBand2", "The system bandwidth to be used in band 2", bandwidthBand2);
  cmd.AddValue ("totalTxPower", "total tx power that will be proportionally assigned to"
                " bands, CCs and bandwidth parts depending on each BWP bandwidth ", totalTxPower);
  cmd.AddValue ("simTag", "tag to be appended to output filenames to distinguish simulation campaigns", simTag);
  cmd.AddValue ("outputDir", "directory where to store simulation results", outputDir);
  cmd.Parse (argc, argv); // 명령 줄 구문 분석


  /*
   * 주파수가 허용 범위에 있는지 확인한다.
   * 다른 확인을 추가해야 한다면, 다음과 같이 하는 것이 가장 좋습니다.
   */
  NS_ABORT_IF (centralFrequencyBand1 > 100e9);
  NS_ABORT_IF (centralFrequencyBand2 > 100e9);


  /*
   * 기록 변수가 true로 설정될 경우 코드를 통해 일부 구성 요소의 로그를 활성화합니다.
   *
   * NS_LOG 환경 변수를 사용해도 동일한 효과를 얻을 수 있습니다:
   * export NS_LOG="UdpClient=level_info|prefix_time|prefix_func|prefix_node:UdpServer=..."
   *
   * 일반적으로 환경 가변 방식은 더 커스터마이징할 수 있고 표현력이 뛰어나기 때문에 선호됩니다.
   */
  if (logging){
    LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
    LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);
  }


  /*
   * 시뮬레이션의 기본값이다.
   * SetDefault의 모든 인스턴스를 점진적으로 제거하고 있지만 레거시 코드인 LTE 코드가 필요하다.
   */
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));


  /************************************************************ << 네트워크 토폴리지 구역 >> ***********************************************************************
   * gNB와 UE를 설정 할 수 있다. 자세한 사항은 GridScenarioHelper 문서를 참조하여 노드가 어떻게 분배되는지 확인하길 바랍니다.
   * 내가 설정한 네트워크 토폴리지는 중앙에 gNB 1개가 생성되고, 12개의 UE가 무작위 위치에서 생성되고 1 ~ 14m/s로 움직인다.
   */
  // UE 이동성 코드
  NodeContainer gNBNodes;
  NodeContainer ueNodes;

  gNBNodes.Create(gNbNum);
  ueNodes.Create(ueNumPergNb * gNbNum);

  MobilityHelper mobility;

  // 기지국(BS, gNB) 위치 설정
  Ptr<ListPositionAllocator> PositionAlloc = CreateObject<ListPositionAllocator> ();
  PositionAlloc -> Add (Vector (0.0, 0.0, 1.5));  // gNB 좌표 (X,Y,Z)
  mobility.SetPositionAllocator(PositionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(gNBNodes);

  // UE 위치 및 이동성 설정
  mobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator",
                                 "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=10.0]"),
                                 "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=10.0]"));
  mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                            "Bounds", RectangleValue(Rectangle(0, 100, 0, 100)),
                            "Speed", StringValue("ns3::UniformRandomVariable[Min=1.0|Max=14.0]"));
  mobility.Install(ueNodes);

  //*********************************************************************************************************************************************************
  /*
   * 서로 다른 트래픽 유형에 대해 서로 다른 2개의 노드 컨테이너를 생성합니다.
   * ueLowLat에서는 저지연 트래픽을 수신할 UE를 넣고,
   * UeVoiceConainter에서는 음성 트래픽을 수신할 UE를 넣는다.
   */
  NodeContainer ueLowLatContainer, ueVoiceContainer;

  for (uint32_t j = 0; j < ueNodes.GetN (); ++j){
    Ptr<Node> ue = ueNodes.Get (j);
    if (j % 2 == 0){
      ueLowLatContainer.Add (ue);
    }
    else{
      ueVoiceContainer.Add (ue);
    }
  }


  /*
   * 할 것: 시나리오를 보여주는 인쇄 또는 플롯을 추가합니다.
   * NR 모듈을 설정합니다. NR 시뮬레이션에 필요한 다양한 도우미를 만듭니다:
   * - EpcHelper, 핵심 네트워크를 설정합니다.
   * - IdealBeamformingHelper, 빔포밍 부분을 처리합니다.
   * - NrHelper, NR 스택의 다양한 부분을 생성하고 연결하는 작업을 처리합니다.
   */
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();


  // nrHelper에 포인터 넣기
  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  /************************************************************************************************************************************************************
   * 스펙트럼 분할 구역
   * 우리는 각각 하나의 구성 요소 반송파를 포함하는 2개의 작동 대역을 생성하고,
   * 각 CC는 입력 매개 변수에 의해 지정된 주파수를 중심으로 단일 대역폭 부분을 포함한다.
   * 각 스펙트럼 부분 길이도 입력 매개변수에 의해 지정된다.
   * 두 운영 대역 모두 StreetCanyon 채널 모델링을 사용한다.
   */
  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;  // 이 예시에서 두 대역 모두 단일 CC를 가지고 있다.


  // CcBwpHelper의 구성을 만듭니다. SimpleOperationBandConf는 아래를 만듭니다.
  // CC 당 단일 BWP
  CcBwpCreator::SimpleOperationBandConf bandConf1 (centralFrequencyBand1, bandwidthBand1, numCcPerBand, BandwidthPartInfo::UMi_StreetCanyon);
  CcBwpCreator::SimpleOperationBandConf bandConf2 (centralFrequencyBand2, bandwidthBand2, numCcPerBand, BandwidthPartInfo::UMi_StreetCanyon);


  // 생성된 구성을 사용함으로써 작업 대역을 만들 때 입니다.
  OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf1);
  OperationBandInfo band2 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf2);

  /*
   * 구성된 스펙트럼 분할은 다음과 같습니다:
   * ------------Band1--------------|--------------Band2-----------------
   * ------------CC1----------------|--------------CC2-------------------
   * ------------BWP1---------------|--------------BWP2------------------
   *
   * ThreeGppChannelModel의 속성은 여전히 저희 방식으로 설정할 수 없습니다.
   * TODO: Tommaso와 협력하기
   */
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds (0)));
  nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));


  /*
   * 채널 및 경로 손실, 그리고 밴드 1 내부의 다른 작업을 초기화합니다.
   * 필요한 경우, 밴드 구성을 수동으로 수행할 수 있지만, 더 정교한 예를 들 수 있습니다.
   * 현재로서는 이 방법이 모든 스펙트럼 초기화 요구 사항을 처리할 것입니다.
   */
  nrHelper->InitializeOperationBand (&band1);


  //예제에서 사용한 대역폭과 BWP 간에 분배해야 하는 총 전력을 고려하기 시작합니다.
  double x = pow (10, totalTxPower / 10);
  double totalBandwidth = bandwidthBand1;

  
  //단일 대역 시뮬레이션이 아닌 경우, 두 번째 대역에서 전원을 초기화하고 초기화하고 설정합니다.
  if (doubleOperationalBand){
    // 채널 및 경로 손실 등 밴드2 내에서 초기화하기
    nrHelper->InitializeOperationBand (&band2);
    totalBandwidth += bandwidthBand2;
    allBwps = CcBwpCreator::GetAllBwps ({band1, band2});
  }
  else{
    allBwps = CcBwpCreator::GetAllBwps ({band1});
  }

  /*
   * allBwps에는 nrHelper에 필요한 모든 스펙트럼 구성이 포함되어 있습니다.
   *
   * 이 부분에서 속성을 설정할 수 있습니다. 세 가지 종류의 속성을 가질 수 있습니다:
   * (i) 모든 대역폭 부분에 유효하고 모든 노드에 적용되는 매개변수,
   * (ii) 모든 대역폭 부분에 유효하고 일부 노드에만 적용되는 매개변수,
   * (iii) 모든 대역폭 부분에 대해 다른 매개변수 
   * 
   * 접근 방식:
   * (i)의 경우: 도우미를 통해 속성을 구성한 다음 설치합니다.
   * (ii)의 경우: 도우미를 통해 속성을 구성한 다음 첫 번째 노드 세트에 설치합니다.
   *            그런 다음 도우미를 통해 속성을 변경한 다음 다시 설치합니다.
   * (iii)의 경우: 설치한 다음 필요한 포인터를 검색하고 해당 포인터 위에 
   *            "SetAttribute"를 호출하여 속성을 구성합니다.
   */

  Packet::EnableChecking ();
  Packet::EnablePrinting ();


  // (i): 모든 노드에 유효한 속성
  // Beamforming(빔포밍) 방법
  idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));

  // Core latency(핵심 지연 시간)
  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));

  // 모든 UE를 위한 안테나
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // 모든 gNb를 위한 안테나
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  uint32_t bwpIdForLowLat = 0;
  uint32_t bwpIdForVoice = 0;
  if (doubleOperationalBand){
    bwpIdForVoice = 1;
    bwpIdForLowLat = 0;
  }

  // 베어러와 대역폭 부분 간의 gNB 라우팅
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));

  // 베어러와 대역폭 부분 간의 UE 라우팅
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));

  /*
   * 다른 많은 매개 변수가 누락되었습니다.
   * 기본적으로 이 매개 변수를 구성하지 않는 것은 기본 값을 사용하는 것과 동일합니다.
   * 여기에 표시되지 않는 모든 속성의 기본값이 무엇인지 확인하려면 설명서를 살펴보세요.
   */


  //(ii): 노드의 하위 집합에 유효한 속성

  // 이 간단한 예제는 표현할 것이 없습니다!!!

  /*
   * 필요한 속성을 구성했습니다.
   * 모든 NR 스택이 포함된 NetDevices에 대한 포인터를 설치하고 가져옵니다:
   */

  NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gNBNodes, allBwps);
  NetDeviceContainer ueLowLatNetDev = nrHelper->InstallUeDevice (ueLowLatContainer, allBwps);
  NetDeviceContainer ueVoiceNetDev = nrHelper->InstallUeDevice (ueVoiceContainer, allBwps);

  // randomStream을 사용하던 기존 코드
  // randomStream += nrHelper->AssignStreams (enbNetDev, randomStream);
  // randomStream += nrHelper->AssignStreams (ueLowLatNetDev, randomStream);
  // randomStream += nrHelper->AssignStreams (ueVoiceNetDev, randomStream);
  
  Ptr<UniformRandomVariable> randomVar = CreateObject<UniformRandomVariable> ();
  randomVar->SetStream (0); // 기본 스트림 설정
  
  
  // (iii): 노드로 이동하여 노드별로 설정해야 하는 속성을 변경합니다.
  
  // 첫 번째 넷 디바이스인(enbNetDev.Get (0))와 첫 번째 대역폭 부분(0)을 가져와 속성을 설정합니다.
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (numerologyBwp1));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("TxPower", DoubleValue (10 * log10 ((bandwidthBand1 / totalBandwidth) * x)));

  if (doubleOperationalBand){
    // 첫 번째 넷 디바이스인(enbNetDev.Get (0))와 두 번째 대역폭 부분(1)을 가져와 속성을 설정합니다.
    nrHelper->GetGnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("Numerology", UintegerValue (numerologyBwp2));
    nrHelper->GetGnbPhy (enbNetDev.Get (0), 1)->SetTxPower (10 * log10 ((bandwidthBand2 / totalBandwidth) * x));
  }

  // 모든 구성이 완료되면 명시적으로 UpdateConfig ()를 호출합니다.

  for (auto it = enbNetDev.Begin (); it != enbNetDev.End (); ++it){
    DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
  }

  for (auto it = ueLowLatNetDev.Begin (); it != ueLowLatNetDev.End (); ++it){
    DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
  }

  for (auto it = ueVoiceNetDev.Begin (); it != ueVoiceNetDev.End (); ++it){
    DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
  }

  // 여기서부터는 표준 NS3입니다. 차후에는 이 부분에 대해서도 헬퍼를 만들겠습니다.

  // 인터넷을 만들고 UE에 IP 스택을 설치하면 SGW/PGW가 생성되고 단일 RemoteHost가 생성됩니다.
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // 원격 호스트를 pgw에 연결합니다. 라우팅도 설정합니다.
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (2500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueNodes);


  Ipv4InterfaceContainer ueLowLatIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLowLatNetDev));
  Ipv4InterfaceContainer ueVoiceIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueVoiceNetDev));

  // UE들의 기본 게이트웨이 설정
  for (uint32_t j = 0; j < ueNodes.GetN (); ++j){
    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get (j)->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  }

  // 가장 가까운 eNB에 UE들 연결
  nrHelper->AttachToClosestEnb (ueLowLatNetDev, enbNetDev);
  nrHelper->AttachToClosestEnb (ueVoiceNetDev, enbNetDev);

  /*
   * 트래픽 부분. 두 가지 종류의 트래픽을 설치합니다:
   * 각각 특정 소스 포트로 식별되는 low-latency와 voice 트래픽을 설치합니다.
   */
  uint16_t dlPortLowLat = 1234;
  uint16_t dlPortVoice = 1235;

  ApplicationContainer serverApps;

  // sink는 항상 지정된 포트에 귀를 기울입니다.
  UdpServerHelper dlPacketSinkLowLat (dlPortLowLat);
  UdpServerHelper dlPacketSinkVoice (dlPortVoice);

  // 수신 중인 애플리케이션인 서버가 UE에 설치되어 있습니다.
  serverApps.Add (dlPacketSinkLowLat.Install (ueLowLatContainer));
  serverApps.Add (dlPacketSinkVoice.Install (ueVoiceContainer));


  /*
   * CBR 트래픽을 생성하기 위해 사용자가 제공한 매개 변수를 사용하여
   * 다양한 생성기의 속성을 구성합니다.
   *
   * Low-Latency 구성 및 개체 생성:
   */
  UdpClientHelper dlClientLowLat;
  dlClientLowLat.SetAttribute ("RemotePort", UintegerValue (dlPortLowLat));
  dlClientLowLat.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  dlClientLowLat.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeULL));
  dlClientLowLat.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaULL)));

  // Low-Latency 트래픽을 전달할 베어러
  EpsBearer lowLatBearer (EpsBearer::NGBR_LOW_LAT_EMBB);

  // Low-Latency 트래픽을 위한 필터
  Ptr<EpcTft> lowLatTft = Create<EpcTft> ();
  EpcTft::PacketFilter dlpfLowLat;
  dlpfLowLat.localPortStart = dlPortLowLat;
  dlpfLowLat.localPortEnd = dlPortLowLat;
  lowLatTft->Add (dlpfLowLat);


  // Voice 구성 및 개체 생성:
  UdpClientHelper dlClientVoice;
  dlClientVoice.SetAttribute ("RemotePort", UintegerValue (dlPortVoice));
  dlClientVoice.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  dlClientVoice.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeBe));
  dlClientVoice.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaBe)));

  // voice 트래픽을 전달할 베어러
  EpsBearer voiceBearer (EpsBearer::GBR_CONV_VOICE);

  // voice 트래픽을 위한 필터
  Ptr<EpcTft> voiceTft = Create<EpcTft> ();
  EpcTft::PacketFilter dlpfVoice;
  dlpfVoice.localPortStart = dlPortVoice;
  dlpfVoice.localPortEnd = dlPortVoice;
  voiceTft->Add (dlpfVoice);

  
  // 애플리케이션을 설치합니다!
  
  ApplicationContainer clientApps;

  for (uint32_t i = 0; i < ueLowLatContainer.GetN (); ++i){
    Ptr<Node> ue = ueLowLatContainer.Get (i);
    Ptr<NetDevice> ueDevice = ueLowLatNetDev.Get (i);
    Address ueAddress = ueLowLatIpIface.GetAddress (i);

    // 전송 중인 클라이언트는 원격 호스트에 설치되며 목적지 주소는 UE의 주소로 설정됩니다.
    dlClientLowLat.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
    clientApps.Add (dlClientLowLat.Install (remoteHost));

    // 트래픽 유형에 대한 전용 베어러 활성화
    nrHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
  }

  for (uint32_t i = 0; i < ueVoiceContainer.GetN (); ++i){
    Ptr<Node> ue = ueVoiceContainer.Get (i);
    Ptr<NetDevice> ueDevice = ueVoiceNetDev.Get (i);
    Address ueAddress = ueVoiceIpIface.GetAddress (i);

    // 전송 중인 클라이언트는 원격 호스트에 설치되며 목적지 주소는 UE의 주소로 설정됩니다.
    dlClientVoice.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
    clientApps.Add (dlClientVoice.Install (remoteHost));

    // 트래픽 유형에 대한 전용 베어러 활성화
    nrHelper->ActivateDedicatedEpsBearer (ueDevice, voiceBearer, voiceTft);
  }

  // UDP 서버 및 클라이언트 앱 시작
  serverApps.Start (udpAppStartTime);
  clientApps.Start (udpAppStartTime);
  serverApps.Stop (simTime);
  clientApps.Stop (simTime);

  // nr 모듈에서 제공하는 추적 활성화
  // nrHelper->EnableTraces();

  FlowMonitorHelper flowmonHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add (remoteHost);
  endpointNodes.Add (ueNodes);

  Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
  monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));

  Simulator::Stop (simTime);
  Simulator::Run ();

  /*
   * 메모리에 설치된 것, 즉, eNb 장치의 BWP들과 그 구성을 확인합니다.
   * 예시 : Node 1 -> Device 0 -> BandwidthPartMap -> {0,1} BWPs -> NrGnbPhy -> Numerology,
  GtkConfigStore config;
  config.ConfigureAttributes ();
  */

  // 흐름별 통계 출력
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

  double averageFlowThroughput = 0.0;
  double averageFlowDelay = 0.0;

  std::ofstream outFile;
  std::string filename = outputDir + "/" + simTag;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::trunc);
  if (!outFile.is_open ()){
    std::cerr << "Can't open file " << filename << std::endl;
    return 1;
  }

  outFile.setf (std::ios_base::fixed);

  double flowDuration = (simTime - udpAppStartTime).GetSeconds ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i){
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
    std::stringstream protoStream;
    protoStream << (uint16_t) t.protocol;
    if (t.protocol == 6){
      protoStream.str ("TCP");
    }
    if (t.protocol == 17){
      protoStream.str ("UDP");
    }
    outFile << "Flow " << i->first << " (" << t.sourceAddress << ":" << t.sourcePort << " -> " << t.destinationAddress << ":" << t.destinationPort << ") proto " << protoStream.str () << "\n";
    outFile << "  Tx Packets: " << i->second.txPackets << "\n";
    outFile << "  Tx Bytes:   " << i->second.txBytes << "\n";
    outFile << "  TxOffered:  " << i->second.txBytes * 8.0 / flowDuration / 1000.0 / 1000.0  << " Mbps\n";
    outFile << "  Rx Bytes:   " << i->second.rxBytes << "\n";
    if (i->second.rxPackets > 0){
      // 수신기 관점에서 흐름의 지속 시간 측정
      averageFlowThroughput += i->second.rxBytes * 8.0 / flowDuration / 1000 / 1000;
      averageFlowDelay += 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets;

      outFile << "  Throughput: " << i->second.rxBytes * 8.0 / flowDuration / 1000 / 1000  << " Mbps\n";
      outFile << "  Mean delay:  " << 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets << " ms\n";
      //outFile << "  Mean upt:  " << i->second.uptSum / i->second.rxPackets / 1000/1000 << " Mbps \n";
      outFile << "  Mean jitter:  " << 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets  << " ms\n";
    }
    else{
      outFile << "  Throughput:  0 Mbps\n";
      outFile << "  Mean delay:  0 ms\n";
      outFile << "  Mean jitter: 0 ms\n";
    }
    outFile << "  Rx Packets: " << i->second.rxPackets << "\n";

    // outFile << "  UE Speed:   " << i->second. / i->second. << " m/s\n";
  }

  outFile << "\n\n  Mean flow throughput: " << averageFlowThroughput / stats.size () << "\n";
  outFile << "  Mean flow delay: " << averageFlowDelay / stats.size () << "\n";

  outFile.close ();

  std::ifstream f (filename.c_str ());

  if (f.is_open ())
    {
      std::cout << f.rdbuf ();
    }

  Simulator::Destroy ();
  return 0;
}
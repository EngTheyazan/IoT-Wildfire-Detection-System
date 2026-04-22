#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/rpl-module.h"
#include "ns3/applications-module.h"
#include "ns3/energy-module.h"
#include "ns3/basic-energy-source.h"
#include "ns3/simple-device-energy-model.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WildfireDetectionSimulation");

// Global variables for metrics
double g_detectionLatency = 0.0;
double g_totalEnergyConsumption = 0.0;
double g_totalPacketsSent = 0.0;
double g_totalPacketsReceived = 0.0;

// Fire event parameters
Vector g_fireLocation;
double g_fireStartTime = 0.0;
double g_fireRadius = 50.0; // meters
bool g_fireActive = false;

// Sensor node application
class SensorApp : public Application
{
public:
  SensorApp ();
  virtual ~SensorApp ();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, double sensingInterval, double fireThreshold);
  void StartApplication ();
  void StopApplication ();

private:
  void SendPacket ();
  void SenseEnvironment ();

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  double          m_sensingInterval;
  double          m_fireThreshold; // Threshold for fire detection
  EventId         m_sendEvent;
  EventId         m_senseEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
  Ptr<Node>       m_node;
  bool            m_fireDetected;
};

SensorApp::SensorApp ()
  : m_socket (0),
    m_peer (Ipv6Address ()),
    m_packetSize (0),
    m_sensingInterval (0.0),
    m_fireThreshold (0.0),
    m_sendEvent (EventId ()),
    m_senseEvent (EventId ()),
    m_running (false),
    m_packetsSent (0),
    m_node (0),
    m_fireDetected (false)
{
}

SensorApp::~SensorApp ()
{
  m_socket = 0;
}

void
SensorApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, double sensingInterval, double fireThreshold)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_sensingInterval = sensingInterval;
  m_fireThreshold = fireThreshold;
}

void
SensorApp::StartApplication ()
{
  m_running = true;
  m_packetsSent = 0;
  m_node = GetNode ();
  m_fireDetected = false;
  m_senseEvent = Simulator::Schedule (Seconds (0.0), &SensorApp::SenseEnvironment, this);
}

void
SensorApp::StopApplication ()
{
  m_running = false;
  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }
  if (m_senseEvent.IsRunning ())
    {
      Simulator::Cancel (m_senseEvent);
    }
  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
SensorApp::SendPacket ()
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->SendTo (packet, 0, m_peer);
  g_totalPacketsSent++;
  m_packetsSent++;
  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s sensor node " << m_node->GetId () << " sent " << m_packetSize << " bytes to " << m_peer);
  if (m_running)
    {
      m_sendEvent = Simulator::Schedule (Seconds (m_sensingInterval), &SensorApp::SendPacket, this);
    }
}

void
SensorApp::SenseEnvironment ()
{
  if (!m_running)
    {
      return;
    }

  // Simulate sensing environmental parameters
  double sensedValue = 25.0; // Normal temperature
  Vector nodeLocation = m_node->GetObject<MobilityModel> ()->GetPosition ();

  if (g_fireActive)
    {
      double distance = nodeLocation.Distance (g_fireLocation);
      if (distance <= g_fireRadius)
        {
          sensedValue = 50.0 + (g_fireRadius - distance); // Higher temperature near fire
        }
    }

  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s sensor node " << m_node->GetId () << " sensed value: " << sensedValue);

  if (sensedValue >= m_fireThreshold && !m_fireDetected)
    {
      m_fireDetected = true;
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s sensor node " << m_node->GetId () << " detected fire!");
      // Send alert packet immediately
      SendPacket ();
      // Record detection latency if this is the first detection for this fire event
      if (g_detectionLatency == 0.0)
        {
          g_detectionLatency = Simulator::Now ().GetSeconds () - g_fireStartTime;
          NS_LOG_INFO ("Fire detection latency: " << g_detectionLatency << "s");
        }
    }
  else if (!m_fireDetected)
    {
      // If no fire detected, continue sensing periodically
      m_senseEvent = Simulator::Schedule (Seconds (m_sensingInterval), &SensorApp::SenseEnvironment, this);
    }
}

// Sink node application
class SinkApp : public Application
{
public:
  SinkApp ();
  virtual ~SinkApp ();

  void Setup (Ptr<Socket> socket);
  void StartApplication ();
  void StopApplication ();

private:
  void HandleRead (Ptr<Socket> socket);

  Ptr<Socket>     m_socket;
  bool            m_running;
};

SinkApp::SinkApp ()
  : m_socket (0),
    m_running (false)
{
}

SinkApp::~SinkApp ()
{
  m_socket = 0;
}

void
SinkApp::Setup (Ptr<Socket> socket)
{
  m_socket = socket;
  m_socket->SetRecvCallback (MakeCallback (&SinkApp::HandleRead, this));
}

void
SinkApp::StartApplication ()
{
  m_running = true;
}

void
SinkApp::StopApplication ()
{
  m_running = false;
  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
SinkApp::HandleRead (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () > 0)
        {
          g_totalPacketsReceived++;
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s sink received " << packet->GetSize () << " bytes from " << Inet6SocketAddress::ConvertFrom (from).GetIpv6 ());
        }
    }
}

// Energy consumption callback
void
EnergyConsumptionCallback (std::string context, double oldValue, double newValue)
{
  g_totalEnergyConsumption += (oldValue - newValue); // Energy consumed
}

int main (int argc, char *argv[])
{
  uint32_t numNodes = 50;
  double simulationTime = 1000.0;
  double sensingInterval = 30.0;
  double fireThreshold = 40.0;
  double batteryCapacityMah = 1000.0;
  uint32_t packetSize = 100; // bytes

  CommandLine cmd (__FILE__);
  cmd.AddValue ("numNodes", "Number of sensor nodes", numNodes);
  cmd.AddValue ("simulationTime", "Total simulation time in seconds", simulationTime);
  cmd.AddValue ("sensingInterval", "Time between sensor readings in seconds", sensingInterval);
  cmd.AddValue ("fireThreshold", "Temperature threshold for fire detection", fireThreshold);
  cmd.AddValue ("batteryCapacityMah", "Initial battery capacity in mAh", batteryCapacityMah);
  cmd.AddValue ("packetSize", "Size of data packets in bytes", packetSize);
  cmd.Parse (argc, argv);

  // Log setup
  LogComponentEnable ("WildfireDetectionSimulation", LOG_LEVEL_INFO);
  LogComponentEnable ("LrWpanCsmaCa", LOG_LEVEL_WARN);
  LogComponentEnable ("SixLowPanNetDevice", LOG_LEVEL_WARN);
  LogComponentEnable ("RplLbr", LOG_LEVEL_WARN);
  LogComponentEnable ("RplNode", LOG_LEVEL_WARN);
  LogComponentEnable ("RplOption", LOG_LEVEL_WARN);
  LogComponentEnable ("RplRoutingProtocol", LOG_LEVEL_WARN);
  LogComponentEnable ("RplTrickleTimer", LOG_LEVEL_WARN);

  // Create nodes
  NodeContainer nodes;
  nodes.Create (numNodes + 1); // +1 for the sink node

  // Install mobility model (random uniform distribution for sensor nodes, fixed for sink)
  MobilityHelper mobility;
  mobility.SetPositionAllocator (CreateObject<UniformDiscPositionAllocator> ());
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Place sink node at (250, 250, 0) for a 500x500m area
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (250.0, 250.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (nodes.Get (0)); // Sink node is node 0

  // Randomly place sensor nodes
  for (uint32_t i = 1; i <= numNodes; ++i)
    {
      Ptr<ConstantPositionMobilityModel> mob = CreateObject<ConstantPositionMobilityModel> ();
      mob->SetPosition (Vector (Simulator::Get  ()->GetRng ()->GetValue (0.0, 500.0), Simulator::Get ()->GetRng ()->GetValue (0.0, 500.0), 0.0));
      nodes.Get (i)->AggregateObject (mob);
    }

  // Install LR-WPAN devices
  LrWpanHelper lrWpanHelper;
  NetDeviceContainer devices = lrWpanHelper.Install (nodes);

  // Install 6LoWPAN adaptation layer
  SixLowPanHelper sixLowPanHelper;
  NetDeviceContainer sixLowPanDevices = sixLowPanHelper.Install (devices);

  // Install Internet stack (IPv6)
  InternetStackHelper internetStackHelper;
  internetStackHelper.Install (nodes);

  // Install RPL routing protocol
  RplHelper rplHelper;
  rplHelper.Set6LowPanHelper (sixLowPanHelper);
  Ptr<RplRoutingProtocol> rpl = rplHelper.Install (nodes.Get (0)); // Sink node is DODAG root

  // Assign IPv6 addresses
  Ipv6AddressHelper ipv6AddressHelper;
  ipv6AddressHelper.SetBase (Ipv6Address ("2001:db8::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer interfaces = ipv6AddressHelper.Assign (sixLowPanDevices);
  interfaces.SetForwarding (0, true); // Enable forwarding on sink
  interfaces.SetDefaultRouteInAllNodes (0); // Set default route to sink

  // Energy Model
  BasicEnergySourceHelper basicEnergySourceHelper;
  basicEnergySourceHelper.Set ("EnergyBudget", DoubleValue (batteryCapacityMah * 3.6)); // Convert mAh to Joules (approx)
  basicEnergySourceHelper.Set ("Period", TimeValue (Seconds (simulationTime)));
  EnergySourceContainer energySources = basicEnergySourceHelper.Install (nodes);

  LrWpanRadioEnergyModelHelper lrWpanRadioEnergyModelHelper;
  lrWpanRadioEnergyModelHelper.Set ("TxCurrentA", DoubleValue (0.0174)); // Transmit current (A)
  lrWpanRadioEnergyModelHelper.Set ("RxCurrentA", DoubleValue (0.0197)); // Receive current (A)
  lrWpanRadioEnergyModelHelper.Set ("SleepCurrentA", DoubleValue (0.000001)); // Sleep current (A)
  lrWpanRadioEnergyModelHelper.Set ("IdleCurrentA", DoubleValue (0.000001)); // Idle current (A)
  DeviceEnergyModelContainer deviceEnergyModels = lrWpanRadioEnergyModelHelper.Install (devices, energySources);

  // Connect energy source to device energy model
  for (uint32_t i = 0; i < nodes.GetN (); ++i)
    {
      Ptr<BasicEnergySource> es = energySources.Get (i)->GetObject<BasicEnergySource> ();
      es->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback (&EnergyConsumptionCallback));
    }

  // Create applications
  // Sink application
  Ptr<Socket> sinkSocket = Socket::CreateSocket (nodes.Get (0), UdpSocketFactory::GetTypeId ());
  sinkSocket->Bind (Inet6SocketAddress (Ipv6Address::Any (), 9));
  Ptr<SinkApp> sinkApp = CreateObject<SinkApp> ();
  sinkApp->Setup (sinkSocket);
  nodes.Get (0)->AddApplication (sinkApp);
  sinkApp->Start (Seconds (0.0));

  // Sensor applications
  ApplicationContainer sensorApps;
  for (uint32_t i = 1; i <= numNodes; ++i)
    {
      Ptr<Socket> sensorSocket = Socket::CreateSocket (nodes.Get (i), UdpSocketFactory::GetTypeId ());
      Ptr<SensorApp> sensorApp = CreateObject<SensorApp> ();
      sensorApp->Setup (sensorSocket, Inet6SocketAddress (interfaces.GetAddress (0,0).ConvertTo<Ipv6Address> (), 9), packetSize, sensingInterval, fireThreshold);
      nodes.Get (i)->AddApplication (sensorApp);
      sensorApps.Add (sensorApp);
    }
  sensorApps.Start (Seconds (1.0));

  // Schedule fire event
  g_fireLocation = Vector (200.0, 200.0, 0.0); // Example fire location
  g_fireStartTime = 100.0; // Fire starts at 100 seconds
  Simulator::Schedule (Seconds (g_fireStartTime), [] () { g_fireActive = true; NS_LOG_INFO ("Fire event started at " << g_fireLocation); });
  Simulator::Schedule (Seconds (g_fireStartTime + 300.0), [] () { g_fireActive = false; NS_LOG_INFO ("Fire event ended."); });

  // Run simulation
  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();
  Simulator::Destroy ();

  // Calculate and print metrics
  double pdr = (g_totalPacketsSent > 0) ? (g_totalPacketsReceived / g_totalPacketsSent) * 100.0 : 0.0;
  NS_LOG_UNCOND ("\n--- Simulation Results ---");
  NS_LOG_UNCOND ("Number of Nodes: " << numNodes);
  NS_LOG_UNCOND ("Simulation Time: " << simulationTime << "s");
  NS_LOG_UNCOND ("Detection Latency: " << g_detectionLatency << "s");
  NS_LOG_UNCOND ("Total Energy Consumption: " << g_totalEnergyConsumption << " Joules");
  NS_LOG_UNCOND ("Packet Delivery Ratio (PDR): " << pdr << "%");

  return 0;
}



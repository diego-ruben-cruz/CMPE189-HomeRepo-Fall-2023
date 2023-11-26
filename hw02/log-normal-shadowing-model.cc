/**
 * Author:
 *
 */

#include "ns3/propagation-loss-model.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

// Borrowing some of the conventions of making a class from propagation-loss-model.cc
class LogNormalShadowingModel : public PropagationLossModel
{
public:
    LogNormalShadowingModel();
    virtual ~LogNormalShadowingModel();

    static TypeId GetTypeId();

    // overriding default methods to add Gaussian Random Var to the mix
    virtual double GetLoss(double txPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b);
    virtual Ptr<PropagationLossModel> Copy() const;

    void SetPathLossExponent(double exponent);
    void SetNoiseVariance(double variance);

private:
    // internal-use vars for loss exponent and noise variance
    double m_pathLossExponent;
    double m_noiseVariance;
};

// Making implementation for the previously-created class, still using propagation-loss-model.cc
NS_OBJECT_ENSURE_REGISTERED(LogNormalShadowingModel);

TypeId LogNormalShadowingModel::GetTypeId()
{
    static TypeId tid = TypeId("LogNormalShadowingModel")
                            .SetParent<PropagationLossModel>()
                            .SetGroupName("Propagation")
                            .AddConstructor<LogNormalShadowingModel>()
                            .AddAttribute("Exponent",
                                          "The exponent of the Path loss propagation model",
                                          DoubleValue(2.8),
                                          MakeDoubleAccessor(&LogNormalShadowingModel::m_pathLossExponent),
                                          MakeDoubleChecker<double>())
                            .AddAttribute("Noise Variance", "The random variable to represent noise",
                                          DoubleValue(4),
                                          MakeDoubleAccessor(&LogNormalShadowingModel::m_noiseVariance),
                                          MakeDoubleChecker<double>());
    return tid;
}

double LogNormalShadowingModel::GetLoss(double txPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b)
{
    // Create LogDistancePropagationLossModel internally, to later add the Gauss random var
    Ptr<LogDistancePropagationLossModel> logDistLoss = CreateObject<LogDistancePropagationLossModel>();
    logDistLoss->SetPathLossExponent(m_pathLossExponent);

    // Calculate path loss thru the conventional method
    double pathLoss = logDistLoss->CalcRxPower(txPowerDbm, a, b);

    // Generate a random variable with log-normal distribution
    Ptr<NormalRandomVariable> normalRandVar = CreateObject<NormalRandomVariable>();
    normalRandVar->SetAttribute("Mean", DoubleValue(0.0));
    normalRandVar->SetAttribute("Variance", DoubleValue(m_noiseVariance));

    // Add the log-normal shadowing effect
    double shadowEffect = normalRandVar->GetValue();
    double totalLoss = pathLoss + shadowEffect;

    return totalLoss;
}
Ptr<PropagationLossModel> LogNormalShadowingModel::Copy() const
{
    return CreateObject<LogNormalShadowingModel>(*this);
}

void LogNormalShadowingModel::SetPathLossExponent(double exponent)
{
    m_pathLossExponent = exponent;
}

void LogNormalShadowingModel::SetNoiseVariance(double variance)
{
    m_noiseVariance = variance;
}

// This is where the simulator will be run with proper cmdline args, consider incomplete
// ...

int main(int argc, char *argv[])
{
    // Borrowed some lines from mythird-hw01.cc and myfirst.cc
    uint32_t nPackets = 1;
    double_t lossExp = 2.5;
    double_t noiseVar = 2;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nPackets", "Number of packets to echo", nPackets);
    cmd.AddValue("lossExp", "The loss exponent", lossExp);
    cmd.AddValue("noiseVar", "The random variable for the noise", noiseVar);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    NS_LOG_INFO("Creating Topology"); // new line added in chp06, useful for adding comments on logs for specific phases of the code

    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper pointToPoint;

    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    MobilityHelper mobility;

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(5.0),
                                  "DeltaY",
                                  DoubleValue(10.0),
                                  "GridWidth",
                                  UintegerValue(3),
                                  "LayoutType",
                                  StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(nPackets));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
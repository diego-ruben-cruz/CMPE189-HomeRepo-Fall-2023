/**
 * Author:
 *
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/propagation-loss-model.h"

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

TypeId
LogNormalShadowingModel::GetTypeId()
{
    static TypeId tid =
        TypeId("LogNormalShadowingModel")
            .SetParent<PropagationLossModel>()
            .SetGroupName("Propagation")
            .AddConstructor<LogNormalShadowingModel>()
            .AddAttribute("Exponent",
                          "The exponent of the Path loss propagation model",
                          DoubleValue(2.8),
                          MakeDoubleAccessor(&LogNormalShadowingModel::m_pathLossExponent),
                          MakeDoubleChecker<double>())
            .AddAttribute("Noise Variance",
                          "The random variable to represent noise",
                          DoubleValue(4),
                          MakeDoubleAccessor(&LogNormalShadowingModel::m_noiseVariance),
                          MakeDoubleChecker<double>());
    return tid;
}

double
LogNormalShadowingModel::GetLoss(double txPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b)
{
    // Create LogDistancePropagationLossModel internally, to later add the Gauss random var
    Ptr<LogDistancePropagationLossModel> logDistLoss =
        CreateObject<LogDistancePropagationLossModel>();
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

Ptr<PropagationLossModel>
LogNormalShadowingModel::Copy() const
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

int main(int argc, char *argv[])
{
    // borrowing some lines from mythird-hw01.cc
    double lossExp = 2.8;
    double noiseVar = 4;

    CommandLine cmd(__FILE__);
    cmd.AddValue("lossExp", "The loss exponent", lossExp);
    cmd.AddValue("noiseVar", "The random variable for the noise", noiseVar);

    cmd.Parse(argc, argv);

    // Create nodes
    NodeContainer nodes;
    nodes.Create(2);

    // Set fixed positions for the nodes
    Ptr<MobilityModel> mobilityA = CreateObject<ConstantPositionMobilityModel>();
    Ptr<MobilityModel> mobilityB = CreateObject<ConstantPositionMobilityModel>();

    nodes.Get(0)->AggregateObject(mobilityA);
    nodes.Get(1)->AggregateObject(mobilityB);

    // Set positions
    Vector posA(0, 0, 0);
    Vector posB(100, 0, 0);

    mobilityA->SetPosition(posA);
    mobilityB->SetPosition(posB);

    // Create point-to-point channel
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices = pointToPoint.Install(nodes);

    // Need to set the model here then do the path loss computing

    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

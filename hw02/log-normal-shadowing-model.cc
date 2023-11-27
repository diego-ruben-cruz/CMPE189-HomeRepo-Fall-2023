/**
 * Author: Diego R Cruz
 *
 */

#include "log-normal-shadowing-model.h"

#include "ns3/boolean.h"
#include "ns3/core-module.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/pointer.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/string.h"

#include <cmath>

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("LogNormalShadowingModel");

// Making implementation for the previously-created class, still using propagation-loss-model.cc
NS_OBJECT_ENSURE_REGISTERED(LogNormalShadowingModel);

TypeId
LogNormalShadowingModel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::LogNormalShadowingModel")
            .SetParent<PropagationLossModel>()
            .SetGroupName("Propagation")
            .AddConstructor<LogNormalShadowingModel>()
            .AddAttribute("pathLossExponent",
                          "The exponent of the Path Loss propagation model",
                          DoubleValue(2.5), // hard-coded to 2.5 for this hw assignment
                          MakeDoubleAccessor(&LogNormalShadowingModel::m_pathLossExponent),
                          MakeDoubleChecker<double>())
            .AddAttribute("refDistance",
                          "The distance at which the reference loss is calculated (m)",
                          DoubleValue(1.0),
                          MakeDoubleAccessor(&LogNormalShadowingModel::m_refDistance),
                          MakeDoubleChecker<double>())
            .AddAttribute("refLoss",
                          "The reference loss at reference distance (dB). (Default is Friis at 1m "
                          "with 5.15 GHz)",
                          DoubleValue(46.6777),
                          MakeDoubleAccessor(&LogNormalShadowingModel::m_refLoss),
                          MakeDoubleChecker<double>())
            .AddAttribute("gaussRandomVar",
                          "The random Gaussian Variable",
                          StringValue("ns3::NormalRandomVariable[Mean=0|Variance=0]"),
                          MakePointerAccessor(&LogNormalShadowingModel::m_gaussRandomVariable),
                          MakePointerChecker<RandomVariableStream>());

    return tid;
}

LogNormalShadowingModel::LogNormalShadowingModel()
{
}

void
LogNormalShadowingModel::SetPathLossExponent(double n)
{
    m_pathLossExponent = n;
}

void
LogNormalShadowingModel::SetReference(double referenceDistance, double referenceLoss)
{
    m_refDistance = referenceDistance;
    m_refLoss = referenceLoss;
}

double
LogNormalShadowingModel::GetPathLossExponent() const
{
    return m_pathLossExponent;
}

double
LogNormalShadowingModel::DoCalcRxPower(double txPowerDbm,
                                       Ptr<MobilityModel> a,
                                       Ptr<MobilityModel> b) const
{
    double distance = a->GetDistanceFrom(b);
    if (distance < -m_refDistance)
    {
        return txPowerDbm - m_refLoss;
    }

    /**
     * Copying log distance prop loss formula is:
     * rx = 10 * log (Pr0(tx)) - n * 10 * log (d/d0)
     *
     * Pr0: rx power at reference distance d0 (W)
     * d0: reference distance: 1.0 (m)
     * d: distance (m)
     * tx: tx power (dB)
     * rx: dB
     *
     * Which becomes modified to:
     *
     * rx = rx0(tx) - 10 * n * log (d/d0)
     */

    double pathlossDbm = 10 * m_pathLossExponent * std::log10(distance / m_refDistance);
    double receivedPower = -m_refLoss - pathlossDbm;
    double gaussLoss = m_gaussRandomVariable->GetValue();

    NS_LOG_DEBUG("distance=" << distance << "m, reference-attenuation=" << -m_refLoss << "dB, "
                             << "attenuation coefficient=" << receivedPower << "db");
    return txPowerDbm + receivedPower + gaussLoss;
}
}
#ifndef LOG_NORMAL_SHADOWING_MODEL_H
#define LOG_NORMAL_SHADOWING_MODEL_H

#include "ns3/object.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/random-variable-stream.h"

#include <map>

namespace ns3
{

    // Borrowing some of the conventions of making a class from propagation-loss-model.cc
    class LogNormalShadowingModel : public PropagationLossModel
    {
    public:
        static TypeId GetTypeId(); // Returns object TypeId
        LogNormalShadowingModel();

        // Removes the copy() bit and assignment op to avoid wronguse
        LogNormalShadowingModel(const LogNormalShadowingModel &) = delete;
        LogNormalShadowingModel &operator=(const LogNormalShadowingModel &) = delete;

        // SetGet combo for the path loss exponent
        void SetPathLossExponent(double exponent);
        double GetPathLossExponent() const;

        // Sets the ref path loss at a given distance
        void SetReference(double refDistance, double refLoss);

    private:
        // internal-use vars for loss exponent and noise variance
        double m_pathLossExponent; // Exponent provided for the model
        double m_refDistance;      // Initial distance that corresponds to Reference Path Loss
        double m_refLoss;          // Path loss at reference distance
        Ptr<RandomVariableStream> m_gaussRandomVariable;

        double DoCalcRxPower(double txPowerDbm,
                             Ptr<MobilityModel> a,
                             Ptr<MobilityModel> b) const override;

        int64_t DoAssignStreams(int64_t stream) override;

        // Creates a default reference path loss/distance in case they are not provided
        static Ptr<PropagationLossModel> CreateDefaultReference();
    };
} // namespace ns3
#endif
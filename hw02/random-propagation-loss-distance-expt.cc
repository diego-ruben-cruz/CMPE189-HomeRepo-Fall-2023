/*
 * Copyright (c) 2012 Telum (www.telum.ru)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mahima Agumbe Suresh <mahima.as@tamu.edu>
 *
 * Modified by: Diego R Cruz
 */

#include "../src/propagation/model/log-normal-shadowing-model.h"

#include "ns3/boolean.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/double.h"
#include "ns3/gnuplot.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/pointer.h"
#include "ns3/propagation-cache.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

#include <cmath>
#include <fstream>
#include <map>

using namespace ns3;

/// Round a double number to the given number of decimal places/sig-figs
/// e.g. dround(0.69420, 0.1) => 0.7
static double
dround(double number, double precision)
{
	number /= precision;
	if (number >= 0)
	{
		number = floor(number + 0.5);
	}
	else
	{
		number = ceil(number - 0.5);
	}
	number *= precision;
	return number;
}

static Gnuplot2dDataset
TestProbabilistic(Ptr<PropagationLossModel> model, double distance, unsigned int samples = 1000)
{
	Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
	Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel>();

	double txPowerDbm = +15; // dBm

	Gnuplot2dDataset dataset;

	dataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	typedef std::map<double, unsigned int> rxPowerMapType;

	// Take given number of samples from CalcRxPower() and show probability
	// density for discrete distances.
	a->SetPosition(Vector(0.0, 0.0, 0.0));
	b->SetPosition(Vector(distance, 0.0, 0.0));

	rxPowerMapType rxPowerMap;
	for (unsigned int samp = 0; samp < samples; ++samp)
	{
		// CalcRxPower() returns dBm.
		double rxPowerDbm = model->CalcRxPower(txPowerDbm, a, b);

		rxPowerDbm = dround(rxPowerDbm, 1.0);
		rxPowerMap[rxPowerDbm]++;

		Simulator::Stop(Seconds(0.01));
		Simulator::Run();
	}

	for (rxPowerMapType::const_iterator i = rxPowerMap.begin(); i != rxPowerMap.end(); ++i)
	{
		dataset.Add(i->first, (double)i->second / (double)samples);
	}

	return dataset;
}

int main(int argc, char *argv[])
{
	CommandLine cmd;
	cmd.Parse(argc, argv);
	std::ofstream plotFile("output.plt");

	// Set the random seed value
	RngSeedManager::SetSeed(3);

	GnuplotCollection gnuplots("hw2-task02-hp03.pdf"); // Change as needed for different parts for task02

	{
		Gnuplot plot;
		plot.AppendExtra("set xlabel 'rxPower (dBm)'");
		plot.AppendExtra("set ylabel 'Probability'");
		plot.AppendExtra("set key outside");

		// need to alter stuff here
		Ptr<LogNormalShadowingModel> randomProp = CreateObject<LogNormalShadowingModel>();
		randomProp->SetAttribute(
			"gaussRandomVar",
			StringValue(
				"ns3::NormalRandomVariable[Mean=0|Variance=2]")); // modify mean and variance for
																  // task02 hp02
		randomProp->SetPathLossExponent(3);						  // Modify for task02 hp02

		for (double distance = 200.0; distance <= 400.0;
			 distance += 50.0) // modify upper bound between LP and HP
		{
			Gnuplot2dDataset dataset = TestProbabilistic(randomProp, distance);
			// New dataset for each distance. Adds a line to the plot
			std::ostringstream os;
			os << "Distance : " << distance;
			dataset.SetTitle(os.str());
			plot.AddDataset(dataset);
		}

		plot.SetTitle("LogNormalShadowingModel");
		gnuplots.AddPlot(plot);
	}

	gnuplots.GenerateOutput(plotFile);
	plotFile.close();
	// produce clean valgrind
	Simulator::Destroy();
	return 0;
}
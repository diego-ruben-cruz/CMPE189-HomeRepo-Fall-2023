/*
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
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FirstScriptExample");

int main(int argc, char *argv[])
{
    uint32_t nPackets = 1;

    CommandLine cmd;
    cmd.AddValue("nPackets", "Number of packets to echo", nPackets);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    NS_LOG_INFO("Creating Topology"); // new line added in chp06, useful for adding comments on logs for specific phases of the code

    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper pointToPoint;
    // pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps")); // Commented out during chp06
    // pointToPoint.SetChannelAttribute("Delay", StringValue("2ms")); // Commented out during chp06

    // In chp06, one exercise entails setting the UdpEchoClient Attribute MaxPackets to some other value
    /*
        ./ns3 run "scratch/myfirst
        --ns3::PointToPointNetDevice::DataRate=5Mbps
        --ns3::PointToPointChannel::Delay=2ms
        --ns3::UdpEchoClient::MaxPackets=2"
    */

    // For the sake of simplicity, here is the cmd structure for changing data rate and packet size as per task02 LP2
    //  ./ns3 run "scratch/myfirst --ns3::PointToPointNetDevice::DataRate=1Mbps --ns3::PointToPointChannel::Delay=1ms --ns3::UdpEchoClient::MaxPackets=1 --ns3::UdpEchoClient::PacketSize=1024"

    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(60.0)); // Changed the value here to get the response to client hello

    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(nPackets));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    // echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(60.0)); // Changed the value here to get the response to client hello

    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll(ascii.CreateFileStream("myfirst.tr"));
    pointToPoint.EnablePcapAll("myfirst");

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}

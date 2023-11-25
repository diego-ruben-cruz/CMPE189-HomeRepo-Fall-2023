# Utility File

## Useful Commands

Here is a useful set of commands to quickly clean ns3 and build it anew:

```cmd
./ns3 clean
./ns3 configure --build-profile=optimized --enable-examples --enable-tests
./ns3 clean
./ns3 configure --build-profile=debug --enable-examples --enable-tests
./ns3 build
```

Here is an example command set to run a program while also changing a few parameters native to the program.

```cmd
./ns3 run "scratch/myfirst --ns3::PointToPointNetDevice::DataRate=1Mbps --ns3::PointToPointChannel::Delay=1ms --ns3::UdpEchoClient::MaxPackets=1 --ns3::UdpEchoClient::PacketSize=1024"
```

Note that this is all on one line, and in no way should be separated by linebreaks.

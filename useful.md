# Utility File

## Useful Commands
Here is a useful set of commands to quickly clean ns3 and build it anew:
```
./ns3 clean
./ns3 configure --build-profile=optimized --enable-examples --enable-tests
./ns3 clean
./ns3 configure --build-profile=debug --enable-examples --enable-tests
./ns3 build
```
# What does this code do ?
This code has 3 separate entity: an EPID group member (Member), an EPID Provenance Verifier (Verifier) and a Cloud server.
It allows the Member to be verified by the Verifier and then connect to the Cloud server as well as sending files through a blockchain.

# Requirements
Package required: libssl-dev

You will also need a Multichain Blockchain running named "chain1" with a stream "stream1".

# Installation
We make the assumption that you are in the project's root path for the commands below:

1) ./configure
2) make clean
3) make

# How to use
Get at that path __install-dir__/_install/epid-sdk/example
There you'll find the executables member, cloud and verifier. 
The verifier will need to be started with an assigned port in the command line.
The cloud will need to be started with an assigned port in the command line.
The member will need the verifier's hostname, the verifier's port, the cloud's hostname, and finally the cloud's port in that specific order.
